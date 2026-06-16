/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#else
#include <GL/gl3w.h>
#endif

#include "SceneView.h"
#include "Allocator.h"
#include "Graphics.h"
#include "Scene.h"
#include "Controller.h"
#include "GlMath.h"
#include "Wire.h"
#include "Device.h"

#define DEFAULT_DISTANCE 8
#define DEFAULT_MOVE_SPEED 20
#define DEFAULT_ROTATE_SPEED 0.12f
#define PERSPECTIVE_ZOOM_SENSITIVITY 0.12f
#define ORTHOGRAPHIC_ZOOM_SENSITIVITY 0.10f
#define DELTA_EPSILON 1E-4

typedef struct scene_view_controller{
    gl_mat projection;
    float fov;
    float znear, zfar;

    float yaw, pitch;
    float move_speed, rot_speed;

    gl_mat rotation;
    gl_vec3 position;

    gl_vec3 offset;
    float distance;
} scene_view_controller;

typedef struct scene_view{
    gfx_framebuffer_handle fbo;        /* resolve FBO (GFX handle, holds color_tex) */
    gfx_texture_handle color_tex;      /* single-sample resolved color — ImGui reads this */
    gfx_texture_handle depth_tex;      /* depth texture (owned by gfx fbo) */
    int32_t width, height;

    /* 4× MSAA render surface — scene renders here, blitted to color_tex each frame */
    uint32_t msaa_fbo;
    uint32_t msaa_color_rbo;
    uint32_t msaa_depth_rbo;
    uint32_t resolve_fbo_id;           /* raw GL FBO that wraps color_tex for blit dst */
    wire_handle wire;
    scene_view_controller controller;
    char const* name;

    /*
     * Granular dirty flags — render triggers:
     *   camera_dirty   : camera controller moved, FOV/type/wireframe changed
     *   viewport_dirty : FBO was resized
     *   scene_dirty    : external scene change (manipulator moved a node)
     *
     * Re-render when any flag is set; clear all after render.
     * shadow_dirty lives in the Window run-loop (shadows belong to the scene/light,
     * not to a particular viewport).
     */
    bool camera_dirty;
    bool viewport_dirty;
    bool scene_dirty;

    bool gizmos_visible;
    bool wireframe;
    enum scene_view_type view_type;
} scene_view;

static float color[4] = {0.24, 0.24, 0.24, 1};

void scene_view_controller_update_projection(scene_view_handle handle) {

    float ar = (float) handle->width / handle->height;

    if (handle->view_type == SCENE_VIEW_PERSPECTIVE) {

        handle->controller.projection = gl_mat_perspective(handle->controller.fov, ar, handle->controller.znear,
                                                           handle->controller.zfar);
    } else {
        float fov = handle->controller.fov / 16;
        handle->controller.projection = gl_mat_ortographic(-fov, fov, -fov * ar, fov * ar, handle->controller.znear,
                                                           handle->controller.zfar);
    }
}

void scene_view_controller_update_internal(scene_view_handle handle) {

    gl_mat mat_yaw = gl_mat_rotate_y(-handle->controller.yaw);
    gl_mat mat_pitch = gl_mat_rotate_x(handle->controller.pitch);
    handle->controller.rotation = gl_mat_mul(mat_yaw, mat_pitch);

    gl_vec3 rel_pos = gl_mat_mul_vec(handle->controller.rotation, gl_vec3_new(0, 0, handle->controller.distance));
    handle->controller.position = gl_vec3_add(handle->controller.offset, rel_pos);
}

void create_fbo(scene_view* handle, int32_t width, int32_t height) {

    /* ---- Resolve target: regular single-sample textures ---- */
    handle->color_tex = gfx_texture_create(width, height, 0, GFX_TEXTURE_TYPE_RGB, GFX_TEXTURE_FILTER_LINEAR,
                                           GFX_TEXTURE_WRAP_CLAMP);
    handle->depth_tex = gfx_texture_create(width, height, 0, GFX_TEXTURE_TYPE_DEPTH, GFX_TEXTURE_FILTER_LINEAR,
                                           GFX_TEXTURE_WRAP_CLAMP);
    handle->fbo = gfx_framebuffer_create(handle->color_tex, handle->depth_tex);

    /* ---- 4× MSAA render FBO ---- */
    glGenRenderbuffers(1, &handle->msaa_color_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, handle->msaa_color_rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB8, width, height);

    glGenRenderbuffers(1, &handle->msaa_depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, handle->msaa_depth_rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, width, height);

    glGenFramebuffers(1, &handle->msaa_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, handle->msaa_fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, handle->msaa_color_rbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_RENDERBUFFER, handle->msaa_depth_rbo);

    /* ---- Resolve FBO: plain GL FBO that wraps color_tex as blit destination ---- */
    glGenFramebuffers(1, &handle->resolve_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, handle->resolve_fbo_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           (uint32_t)gfx_texture_get_id(handle->color_tex), 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

const char* view_type_to_string(scene_view_type view_type) {
    switch (view_type) {

        case SCENE_VIEW_PERSPECTIVE:
            return "SCENE_VIEW_PERSPECTIVE";
        case SCENE_VIEW_ORTOGRAPHIC_FRONT:
            return "SCENE_VIEW_ORTOGRAPHIC_FRONT";
        case SCENE_VIEW_ORTOGRAPHIC_SIDE:
            return "SCENE_VIEW_ORTOGRAPHIC_SIDE";
    }
    return "DEFAULT";
}

static void scene_view_controller_apply_view_type(scene_view_handle handle) {
    switch (handle->view_type) {
        case SCENE_VIEW_PERSPECTIVE:
        case SCENE_VIEW_ORTOGRAPHIC_FRONT:
            handle->controller.yaw = 0.0f;
            handle->controller.pitch = 0.0f;
            break;
        case SCENE_VIEW_ORTOGRAPHIC_SIDE:
            handle->controller.yaw = -90.0f;
            handle->controller.pitch = 0.0f;
            break;
    }

    scene_view_controller_update_internal(handle);
}

scene_view_handle scene_view_create(int32_t width, int32_t height, scene_view_type view_type)
{
    scene_view_handle handle = OS_MALLOC(sizeof(struct scene_view));
    os_memset(handle, 0, sizeof(struct scene_view));

    handle->controller.fov = 60;
    handle->controller.zfar = 1000;
    handle->controller.znear = 0.01;
    handle->view_type = view_type;
    handle->name = view_type_to_string(view_type);
    handle->width = width;
    handle->height = height;
    handle->camera_dirty = true;
    handle->gizmos_visible = true;
    handle->wireframe = false;
    handle->wire = wire_new(512);

    create_fbo(handle, width, height);

    switch (view_type) {

        case SCENE_VIEW_PERSPECTIVE: {
            wire_grid(handle->wire, 32, 0);
        }
            break;
        case SCENE_VIEW_ORTOGRAPHIC_FRONT: {
            wire_grid(handle->wire, 32, 2);
        }
            break;
        case SCENE_VIEW_ORTOGRAPHIC_SIDE: {
            wire_grid(handle->wire, 32, 2);
        }
            break;
    }

    wire_apply(handle->wire);

    handle->controller.rotation = gl_mat_new_identity();
    handle->controller.distance = DEFAULT_DISTANCE;
    handle->controller.move_speed = DEFAULT_MOVE_SPEED;
    handle->controller.rot_speed = DEFAULT_ROTATE_SPEED;

    scene_view_controller_apply_view_type(handle);
    scene_view_controller_update_projection(handle);

    /* Trigger a full render on the first frame */
    handle->camera_dirty   = true;
    handle->viewport_dirty = false;
    handle->scene_dirty    = false;

    return handle;
}

void scene_view_update_controller(scene_view_handle handle) {

    device_joystick joystick = device_joystick_get();
    bool has_changed_position = false;
    bool has_changed_projection = false;

    if (gl_abs(joystick.pointer.dy) > DELTA_EPSILON ||
        gl_abs(joystick.pointer.dx) > DELTA_EPSILON) {

        if (handle->view_type == SCENE_VIEW_PERSPECTIVE && joystick.mouse.rmb_down) {

            handle->controller.yaw += (float)joystick.pointer.dx * handle->controller.rot_speed;
            handle->controller.pitch += (float)joystick.pointer.dy * handle->controller.rot_speed;
            handle->controller.pitch = gl_clamp(handle->controller.pitch, -89.0f, 89.0f);
            has_changed_position = true;
        }

        if (joystick.mouse.mmb_down) {
            /* Pan is driven by mouse pixel deltas which already scale with 1/FPS,
               so multiplying by dt would make speed proportional to 1/FPS^2.
               Use a fixed scale matching the original 75 FPS feel (dt ≈ 1/75). */
            const float pan_scale = 1.0f / 75.0f;
            gl_vec3 move = gl_vec3_new(-joystick.pointer.dx * pan_scale,
                                       joystick.pointer.dy * pan_scale, 0);
            handle->controller.offset = gl_vec3_add(handle->controller.offset,
                                                    gl_mat_mul_vec(handle->controller.rotation, move));
            has_changed_position = true;
        }
    }

    if (gl_abs(joystick.pointer.scroll_dy) > DELTA_EPSILON) {
        if (handle->view_type == SCENE_VIEW_PERSPECTIVE) {
            handle->controller.distance *= expf(-(float)joystick.pointer.scroll_dy * PERSPECTIVE_ZOOM_SENSITIVITY);
            handle->controller.distance = gl_clamp(handle->controller.distance, 0.2f, 100.0f);
            has_changed_position = true;
        }
        else {
            handle->controller.fov *= expf(-(float)joystick.pointer.scroll_dy * ORTHOGRAPHIC_ZOOM_SENSITIVITY);
            handle->controller.fov = gl_clamp(handle->controller.fov, 1.0f, 200.0f);
            has_changed_projection = true;
        }
    }

    if(has_changed_position)
    {
        scene_view_controller_update_internal(handle);
        handle->camera_dirty = true;
    }

    if(has_changed_projection){
        scene_view_controller_update_projection(handle);
        handle->camera_dirty = true;
    }
}

void scene_view_render(scene_view_handle handle, void* scene) {
    if (handle->camera_dirty || handle->viewport_dirty || handle->scene_dirty) {

        /* ---- 1. Render scene into 4× MSAA FBO ---- */
        glBindFramebuffer(GL_FRAMEBUFFER, handle->msaa_fbo);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(color[0], color[1], color[2], color[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, handle->width, handle->height);

        gl_mat tr = handle->controller.rotation;
        tr = gl_mat_set_translation(tr, handle->controller.position);

        scene_draw_with_camera(scene,
                               handle->controller.projection.data,
                               tr.data,
                               handle->view_type == SCENE_VIEW_PERSPECTIVE,
                               handle->wireframe);

        if (handle->gizmos_visible) {
            wire_clear_all(handle->wire);
            wire_axis(handle->wire, handle->controller.offset.data);
            wire_draw(handle->wire, handle->controller.projection.data, gl_mat_inverse(tr).data);
        }

        /* ---- 2. Resolve MSAA → single-sample color_tex ---- */
        glBindFramebuffer(GL_READ_FRAMEBUFFER, handle->msaa_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle->resolve_fbo_id);
        glBlitFramebuffer(0, 0, handle->width, handle->height,
                          0, 0, handle->width, handle->height,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);

        /* Restore default FBO so subsequent ImGui/GFX calls aren't affected */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        handle->camera_dirty   = false;
        handle->viewport_dirty = false;
        handle->scene_dirty    = false;
    }
}

void scene_view_render_get(scene_view_handle handle, int32_t* color, int32_t* depth) {
    if (handle->color_tex != 0 ) {
        if(color != 0)
            *color = gfx_texture_get_id(handle->color_tex);
    }else {
        if(color != 0)
            *color = -1;
    }
    if (handle->depth_tex != 0) {
        if(depth != 0)
            *depth = gfx_texture_get_id(handle->depth_tex);
    }
    else {
        if(depth != 0)
            *depth = -1;
    }
}

void scene_view_resize(scene_view_handle handle, int32_t w, int32_t h) {
    if (handle->width != w || handle->height != h) {

        handle->width = w;
        handle->height = h;
        handle->viewport_dirty = true;
        handle->camera_dirty   = true;  /* projection aspect ratio changed */

        if(w != 0 && h != 0) {
            /* Destroy MSAA resources before GFX resources */
            glDeleteFramebuffers(1,  &handle->msaa_fbo);
            glDeleteFramebuffers(1,  &handle->resolve_fbo_id);
            glDeleteRenderbuffers(1, &handle->msaa_color_rbo);
            glDeleteRenderbuffers(1, &handle->msaa_depth_rbo);

            gfx_texture_destroy(handle->color_tex);
            gfx_texture_destroy(handle->depth_tex);
            gfx_framebuffer_destroy(handle->fbo);
            create_fbo(handle, w, h);
            scene_view_controller_update_projection(handle);
        }

    }
}

void scene_view_get_size(scene_view_handle handle, int32_t* width, int32_t* height){
    *width = handle->width;
    *height = handle->height;
}

scene_view_type scene_view_get_type(scene_view_handle handle) {
    return handle->view_type;
}

void scene_view_set_type(scene_view_handle handle, scene_view_type view_type) {
    if (handle->view_type == view_type) {
        return;
    }

    handle->view_type = view_type;
    handle->name = view_type_to_string(view_type);

    scene_view_controller_apply_view_type(handle);

    scene_view_controller_update_projection(handle);
    handle->camera_dirty = true;
}

float scene_view_get_fov(scene_view_handle handle) {
    return handle->controller.fov;
}

void scene_view_set_fov(scene_view_handle handle, float fov) {
    if (fov < DELTA_EPSILON) {
        fov = DELTA_EPSILON;
    }

    if (gl_abs(handle->controller.fov - fov) <= DELTA_EPSILON) {
        return;
    }

    handle->controller.fov = fov;
    scene_view_controller_update_projection(handle);
    handle->camera_dirty = true;
}

bool scene_view_get_gizmos_visible(scene_view_handle handle) {
    return handle->gizmos_visible;
}

void scene_view_set_gizmos_visible(scene_view_handle handle, bool visible) {
    if (handle->gizmos_visible == visible) {
        return;
    }

    handle->gizmos_visible = visible;
    handle->camera_dirty = true;
}

bool scene_view_get_wireframe(scene_view_handle handle) {
    return handle->wireframe;
}

void scene_view_set_wireframe(scene_view_handle handle, bool enabled) {
    if (handle->wireframe == enabled) {
        return;
    }

    handle->wireframe = enabled;
    handle->camera_dirty = true;
}

void scene_view_destroy(scene_view_handle handle){
    wire_delete(handle->wire);

    glDeleteFramebuffers(1,  &handle->msaa_fbo);
    glDeleteFramebuffers(1,  &handle->resolve_fbo_id);
    glDeleteRenderbuffers(1, &handle->msaa_color_rbo);
    glDeleteRenderbuffers(1, &handle->msaa_depth_rbo);

    gfx_texture_destroy(handle->depth_tex);
    gfx_texture_destroy(handle->color_tex);
    gfx_framebuffer_destroy(handle->fbo);
    OS_FREE(handle);
}

char const* scene_view_get_name(scene_view_handle handle){
    return handle->name;
}

void scene_view_flag_dirty(scene_view_handle handle) {
    handle->scene_dirty = true;
}
