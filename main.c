#include <stdio.h>
#include "Device/Device.h"
#include "Os/Allocator.h"
#include "Graphics/Graphics.h"
#include <Interface/Text.h>
#include <Model/Model.h>
#include <Scene/Scene.h>
#include <Draw/Draw.h>
#include <Interface/Sprite.h>
#include <Shaders/DepthShader.h>

/*
 * Resources.
 */
scene_handle active_scene;

void escape()
{
    device_window_close();
}

void toggle_wireframe()
{
    scene_wireframe_toggle(active_scene);
}

void toggle_bounding_box()
{
    scene_bounding_box_toggle(active_scene);
}


int main()
{


    device_desc device_desc = {
            .height = 720,
            .width = 1280,
            .name = "Device",
            .fullscreen = false,
            .msaa = 0,
    };

    device_init();
    device_cursor_state cursor_desc = {.centered = true, .visible = false};
    device_handle handle = device_new(&device_desc);
    device_set_current(handle);
    device_window_cursor_set(&cursor_desc);
    device_joystick_trigger_register(JOYSTICK_TRIGGER_MENU, JOYSTICK_TRIGGER_PRESS, escape);
    device_joystick_trigger_register(JOYSTICK_TRIGGER_1, JOYSTICK_TRIGGER_PRESS, toggle_wireframe);
    device_joystick_trigger_register(JOYSTICK_TRIGGER_2, JOYSTICK_TRIGGER_PRESS, toggle_bounding_box);
    int32_t width, height;
    device_window_dimensions_get(&width, &height);

    gfx_init();
    text_init();

    sprite_init(8);

    gfx_texture_desc tex_desc = {
            .data = 0,
            .width =  width ,
            .height = height,
            .type = GFX_TEXTURE_TYPE_SRGB,
            .mipmaps = false,
            .filter = GFX_TEXTURE_FILTER_NEAREST
    };
    gfx_texture_desc depth_tex = {
            .data = 0,
            .width =  width ,
            .height = height ,
            .type = GFX_TEXTURE_TYPE_DEPTH,
            .mipmaps = false,
            .filter = GFX_TEXTURE_FILTER_NEAREST
    };

    gfx_texture_handle fbo_color_texture = gfx_texture_create(&tex_desc);
    gfx_texture_handle depth_color_texture = gfx_texture_create(&depth_tex);

    gfx_framebuffer_desc fbo_desc = {
            .color_attachments = {
                    {
                            .enabled = true,
                            .texture_handle = fbo_color_texture,
                    }
            },
            .depth_stencil_attachment = {
                    .texture_handle = depth_color_texture,
                    .enabled = true,
            }
    };



    gfx_framebuffer_handle fbo_handle = gfx_framebuffer_create(&fbo_desc);

    gfx_pass_desc pass_desc = {
            .pass_options = GFX_PASS_OPTION_DEPTH_TEST | GFX_PASS_OPTION_CULL_BACK,
            .actions = GFX_PASS_ACTION_CLEAR_STENCIL | GFX_PASS_ACTION_CLEAR_DEPTH | GFX_PASS_ACTION_CLEAR_COLOR,
            .fbo_handle = fbo_handle,
            .clear_color = {0.2f, 0.8f, 1, 1}
    };

    gfx_pass_desc default_pass = {
            .pass_options = GFX_PASS_OPTION_FRAMEBUFFER_SRGB | GFX_PASS_OPTION_CULL_BACK,
            .actions = GFX_PASS_ACTION_CLEAR_STENCIL | GFX_PASS_ACTION_CLEAR_COLOR,
            .fbo_handle = 0,
            .clear_color = {0, 0, 0, 1}
    };

    mdl_desc sponza_mdl_desc = {
            .path = "./Data/sponza_tex.gltf",
            .load_textures = false
    };

    scene_desc s_desc = {
            .model = mdl_load(&sponza_mdl_desc),
            .enable_shadows = true,
            .lighting = SCENE_DEFAULT_LIGHTING,
            .sun = SCENE_DEFAULT_SUN,
    };
    active_scene = scene_new(&s_desc);
    mdl_unload(s_desc.model);
    gfx_shader_handle depth_shader = gfx_shader_create(&sprite_shader_desc);
    sprite_desc fb1 = {.gfx_texture_handle = fbo_color_texture, .width = width, .height = height, .scale= 1, .offset_y = 0, .offset_x = 0};
    sprite_desc fb2 = {.gfx_texture_handle = scene_get_lighting_depth_texture(active_scene), .width = 300, .height = 200, .scale= 1, .offset_y = 0, .offset_x = 0, .custom_shader = {.enabled = true, .gfx_shader_handle = depth_shader}};
    sprite_new(fb1);
    sprite_new(fb2);

    dw_desc desc = {};
    dw_handle dw = dw_new(&desc, 100);
    dw_grid(dw, 10);
    dw_apply(dw);

    char text_buffer[100];
    text_handle fps_text = text_new(100);
    text_handle alloc_text = text_new(100);
    text_desc fps_text_desc = {.label = text_buffer, .offset_x = 0, .offset_y = height - 30, .scale = 1};
    text_desc alloc_text_desc = {.label = text_buffer, .offset_x = 0, .offset_y = height - 60, .scale = 1};

    scene_camera_projection scene_mvp;
    while(device_window_valid()) {

        device_update_events();
        gfx_reset_draw_call_count();

        //First pass
        gfx_begin_pass(&pass_desc);
        scene_draw(active_scene);
        scene_mvp = scene_camera_projection_get();

        sprintf(text_buffer, "Draw Calls: %i Fps: %i", gfx_draw_call_count_get(), (int)(1.0 / device_dt_get()));
        text_update(fps_text, &fps_text_desc);
        sprintf(text_buffer, "Heap count: %i Heap size: %i", os_get_tracked_allocations_length(), (int)(os_get_tracked_allocations_size()));
        text_update(alloc_text, &alloc_text_desc);
        gfx_end_pass();

        //Second pass
        gfx_begin_pass(&default_pass);
        dw_draw(dw, scene_mvp.projection, scene_mvp.view);
        sprite_draw();
        text_draw(fps_text);
        text_draw(alloc_text);
        gfx_end_pass();

        //Swap buffers
        device_refresh();

    }

    text_delete(fps_text);
    text_delete(alloc_text);

    gfx_framebuffer_destroy(fbo_handle);
    gfx_shader_destroy(depth_shader);
    gfx_texture_destroy(fbo_color_texture);
    gfx_texture_destroy(depth_color_texture);

    text_terminate();
    dw_delete(dw);
    sprite_terminate();
    gfx_terminate();
    scene_delete(active_scene);
    device_delete(handle);
    device_terminate();
    return 0;
}
