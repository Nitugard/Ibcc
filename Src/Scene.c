/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "GlMath.h"
#include "Scene.h"
#include "Allocator.h"
#include "Graphics.h"
#include "Model.h"
#include "Device.h"
#include "Skybox.h"
#include "Shaders/Common.h"
#include "Wire.h"
#include "ShadowRenderer.h"
#include "GroundRenderer.h"
#include "BrdfLut.h"
#include "PrefilterEnv.h"

#include <string.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#else
#include <GL/gl3w.h>
#endif

typedef struct scene_internal_node {
    char* name;

    gl_mat world_tr;
    gl_mat local_tr;

    int32_t local_id;
    int32_t parent_id;

    int32_t * children_id;
    int32_t children_count;
    int32_t mesh_index;
    int32_t camera_index;
    int32_t light_index;
} scene_internal_node;

typedef struct scene_internal_texture{
    gfx_texture_handle texture_handle;
} scene_internal_texture;

typedef struct scene_internal_pbr_material{
    bool valid;
    int32_t color_texture_id;
    float color_factor[4];
    int32_t metalic_texture_id;
    float metallic_factor;
    int32_t rough_texture_id;
    float roughness_factor;

    int32_t model_uniform;
    int32_t projection_uniform;
    int32_t view_uniform;
    int32_t view_position_uniform;
    int32_t rougness_uniform;
    int32_t metallic_uniform;
    int32_t color_uniform;
    int32_t skybox_uniform;
    int32_t exposure_uniform;
    int32_t shadow_map_uniform;
    int32_t light_space_uniform;
    int32_t brdf_lut_uniform;
    int32_t prefiltered_env_uniform;
    int32_t base_color_tex_uniform;
    int32_t has_color_tex_uniform;
    int32_t has_vertex_color_uniform;

    gfx_shader_handle shader;
} scene_internal_pbr_material;

typedef struct scene_internal_mesh_primitive{
    int32_t indices_count;
    int32_t material_id;

    gfx_pipeline_handle pipeline_handle;
    gfx_pipeline_handle shadow_pipeline;
    gfx_pipeline_handle highlight_pipeline;
    gfx_buffer_handle buffer_handle;
    gfx_buffer_handle index_handle;

    gfx_draw_type draw_type;
    bool has_vertex_color;
} scene_internal_mesh_primitive;

typedef struct scene_internal_mesh{
    int32_t node_index;
    int32_t primitives_count;
    scene_internal_mesh_primitive* primitives;
} scene_internal_mesh;

typedef struct scene_internal_light{
    int32_t node_index;
    enum mdl_light_type light_type;
    gl_vec3 color;
    float intensity;
} scene_internal_light;


typedef struct scene_internal_camera{
    int32_t node_index;
    bool ortographic;
    float fov;
    float ar;
    float zfar, znear;
    float xmag, ymag;
    gl_mat projection;
    gl_mat view;
} scene_internal_camera;


typedef struct scene_internal_data{
    uint32_t meshes_count;
    scene_internal_mesh* meshes;

    uint32_t root_nodes_count;
    scene_internal_node** root_nodes;

    uint32_t leaf_nodes_count;
    scene_internal_node** leaf_nodes;

    uint32_t nodes_count;
    scene_internal_node* nodes;

    uint32_t materials_count;
    scene_internal_pbr_material* materials;

    uint32_t textures_count;
    scene_internal_texture* textures;

    uint32_t cameras_count;
    scene_internal_camera* cameras;

    uint32_t lights_count;
    scene_internal_light* lights;

    int32_t active_camera_node;

    skybox_handle skybox;
    bool skybox_enabled;
    bool skybox_render;
    bool plane_render;

    shadow_renderer shadow;
    ground_renderer ground;
    gfx_texture_handle brdf_lut;
    uint32_t prefiltered_env_id;   /* raw GL cubemap — 0 when no skybox */

    /* Selection + highlight */
    int32_t selected_node_id;
    gfx_shader_handle highlight_shader;
    int32_t hl_model_u, hl_view_u, hl_proj_u, hl_color_u, hl_scale_u;
} scene_internal_data;

scene_internal_mesh_primitive scene_new_primitive(gfx_shader_handle shader, gfx_shader_handle shadow_shader, mdl_primitive primitive) {
    scene_internal_mesh_primitive result = {0};
    result.buffer_handle = gfx_buffer_create(GFX_BUFFER_VERTEX, GFX_BUFFER_UPDATE_STATIC_DRAW, primitive.vertices,
                                             primitive.vertex_stride * primitive.vertices_count);

    switch (primitive.primitive_type) {

        case MDL_PRIMITIVE_TYPE_POINTS: result.draw_type = GFX_POINTS;break;
        case MDL_PRIMITIVE_TYPE_LINES: result.draw_type = GFX_LINES;break;
        case MDL_PRIMITIVE_TYPE_LINE_LOOP: result.draw_type = GFX_LINE_LOOP;break;
        case MDL_PRIMITIVE_TYPE_LINE_STRIP: result.draw_type = GFX_LINE_STRIP;break;
        case MDL_PRIMITIVE_TYPE_TRIANGLES: result.draw_type = GFX_TRIANGLES;break;
        case MDL_PRIMITIVE_TYPE_TRIANGLE_STRIP: result.draw_type = GFX_TRIANGLE_STRIP;break;
        case MDL_PRIMITIVE_TYPE_TRIANGLE_FAN: result.draw_type = GFX_TRIANGLE_FAN;break;
    }

    result.draw_type = (uint32_t)primitive.primitive_type;
    result.pipeline_handle = gfx_pipeline_create(shader);

    for (int32_t i = 0; i < primitive.attributes_count; ++i) {

        const char *attribute_name = 0;
        switch (primitive.attributes[i].type) {

            case MDL_VERTEX_ATTRIBUTE_INVALID:
                attribute_name = 0;
                break;
            case MDL_VERTEX_ATTRIBUTE_POSITION:
                attribute_name = ATTR_POSITION_NAME;
                break;
            case MDL_VERTEX_ATTRIBUTE_UV:
                attribute_name = ATTR_UV_NAME;
                break;
            case MDL_VERTEX_ATTRIBUTE_NORMAL:
                attribute_name = ATTR_NORMAL_NAME;
                break;
            case MDL_VERTEX_ATTRIBUTE_COLOR:
                attribute_name = ATTR_COLOR_NAME;
                break;
            case MDL_VERTEX_ATTRIBUTE_TANGENT:
                attribute_name = ATTR_TANGENT_NAME;
                break;
            default:
                attribute_name = 0;
        }

        if (attribute_name != 0) {
            gfx_pipeline_attr_enable(result.pipeline_handle, attribute_name, result.buffer_handle,
                                     primitive.attributes[i].count, primitive.attributes[i].offset,
                                     primitive.vertex_stride);
        }
    }

    gfx_buffer_handle index_handle = gfx_buffer_create(GFX_BUFFER_INDEX, GFX_BUFFER_UPDATE_STATIC_DRAW, primitive.indices,
                                                       primitive.indices_count * sizeof(uint32_t));

    gfx_pipeline_index_enable(result.pipeline_handle, index_handle);
    gfx_pipeline_submit(result.pipeline_handle);

    /* Shadow pipeline — position attribute only */
    result.shadow_pipeline = gfx_pipeline_create(shadow_shader);
    for (int32_t i = 0; i < primitive.attributes_count; ++i) {
        if (primitive.attributes[i].type == MDL_VERTEX_ATTRIBUTE_POSITION) {
            gfx_pipeline_attr_enable(result.shadow_pipeline, ATTR_POSITION_NAME, result.buffer_handle,
                                     primitive.attributes[i].count, primitive.attributes[i].offset,
                                     primitive.vertex_stride);
            break;
        }
    }
    gfx_pipeline_index_enable(result.shadow_pipeline, index_handle);
    gfx_pipeline_submit(result.shadow_pipeline);

    result.indices_count = primitive.indices_count;
    result.index_handle = index_handle;
    result.material_id = primitive.material_id;
    result.has_vertex_color = (primitive.attributes_flag & MDL_VERTEX_ATTRIBUTE_COLOR) != 0;
    return result;
}

scene_handle scene_new(scene_desc const* desc) {
    mdl_data* model = desc->model;

    //create scene structure
    scene_handle handle = OS_MALLOC(sizeof(struct scene_internal_data));
    os_memset(handle, 0, sizeof(scene_internal_data));


    handle->skybox_enabled = desc->skybox.path != 0;
    handle->skybox_render = desc->skybox.render;
    handle->plane_render = true;
    handle->prefiltered_env_id = 0;
    if(desc->skybox.path != 0) {
        handle->skybox = skybox_load(desc->skybox.path);
        handle->prefiltered_env_id = prefilter_env_generate(skybox_get_cubemap_id(handle->skybox));
    }

    /*
     * Loading of the lights.
     */

    handle->lights_count = model->lights_count;
    handle->lights = OS_MALLOC(sizeof(struct scene_internal_light) * model->lights_count);
    for(int32_t i=0; i<handle->lights_count; ++i) {
        scene_internal_light *light = handle->lights + i;
        mdl_light *m_light = model->lights + i;

        light->intensity = m_light->intensity;
        os_memcpy(light->color.data, m_light->color, sizeof(float) * 3);
        light->light_type = m_light->light_type;
    }


    /*
     * Loading of the cameras
     */
    int32_t w,h;
    device_window_dimensions_get(&w, &h);

    handle->cameras_count = model->cameras_count;
    handle->cameras = OS_MALLOC(sizeof(struct scene_internal_camera) * model->cameras_count);

    for (uint32_t i = 0; i < handle->cameras_count; ++i) {
        struct scene_internal_camera *camera = handle->cameras + i;
        mdl_camera *m_cam = model->cameras + i;

        camera->ortographic = m_cam->ortographic;
        camera->xmag = m_cam->xmag;
        camera->zfar = m_cam->zfar;
        camera->znear = m_cam->znear;
        camera->ymag = m_cam->ymag;
        camera->fov = m_cam->fov;
        camera->ar = (float)w / h;
        if (!camera->ortographic) {
            camera->projection = gl_mat_perspective(camera->fov, camera->ar, camera->znear,
                                                    camera->zfar);
        } else {
            camera->projection = gl_mat_ortographic(-camera->fov, camera->fov, -camera->fov / camera->ar,
                                                    camera->fov / camera->ar, camera->znear,
                                                    camera->zfar);
        }
    }

    /*
     * Loading of the materials.
     */
    handle->materials_count = model->materials_count;
    handle->materials = OS_MALLOC(sizeof(struct scene_internal_pbr_material) * model->materials_count);

    void* fs = device_file_read_text("./Shaders/Lit.fs");
    void* vs = device_file_read_text("./Shaders/Lit.vs");

    for (uint32_t i = 0; i < model->materials_count; ++i) {
        struct scene_internal_pbr_material *mat = handle->materials + i;
        struct mdl_material *m_mat = model->materials + i;

        mat->shader = gfx_shader_create("Lit");
        gfx_shader_add_fs(mat->shader, fs);
        gfx_shader_add_vs(mat->shader, vs);
        gfx_shader_submit(mat->shader);

        mat->roughness_factor = m_mat->roughness_factor ;
        mat->metallic_factor = m_mat->metallic_factor;
        os_memcpy(mat->color_factor, m_mat->color_factor, sizeof(float) * 4);
        mat->valid = m_mat->valid;
        mat->color_texture_id = m_mat->color_texture_id;

        gfx_shader_uniform_enable(mat->shader, MODEL_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &mat->model_uniform );
        gfx_shader_uniform_enable(mat->shader, PROJECTION_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &mat->projection_uniform );
        gfx_shader_uniform_enable(mat->shader, VIEW_TRANSFORM_NAME, GFX_TYPE_FLOAT_MAT_4, &mat->view_uniform );
        gfx_shader_uniform_enable(mat->shader, VIEW_POSITION_NAME, GFX_TYPE_FLOAT_VEC_3, &mat->view_position_uniform );
        gfx_shader_uniform_enable(mat->shader, "roughness", GFX_TYPE_FLOAT_VEC_1, &mat->rougness_uniform );
        gfx_shader_uniform_enable(mat->shader, "metallic", GFX_TYPE_FLOAT_VEC_1, &mat->metallic_uniform );
        gfx_shader_uniform_enable(mat->shader, "base_color", GFX_TYPE_FLOAT_VEC_3, &mat->color_uniform);
        gfx_shader_uniform_enable(mat->shader, "skybox", GFX_TYPE_SAMPLER_CUBE, &mat->skybox_uniform);
        gfx_shader_uniform_enable(mat->shader, "exposure", GFX_TYPE_FLOAT_VEC_1, &mat->exposure_uniform);

    }
    OS_FREE(vs);
    OS_FREE(fs);

    /*
     * Upload GLTF images to GPU as sRGBA textures.
     */
    handle->textures_count = (uint32_t)model->textures_count;
    if (model->textures_count > 0) {
        handle->textures = OS_MALLOC(sizeof(scene_internal_texture) * model->textures_count);
        for (int32_t i = 0; i < model->textures_count; ++i) {
            mdl_texture *src = model->textures + i;
            handle->textures[i].texture_handle = 0;
            if (src->valid) {
                handle->textures[i].texture_handle = gfx_texture_create(
                    src->width, src->height, src->buffer,
                    GFX_TEXTURE_TYPE_SRGBA,
                    GFX_TEXTURE_FILTER_LINEAR,
                    GFX_TEXTURE_WRAP_REPEAT);
            }
        }
    }

    /* Register texture + vertex-color uniforms for every Lit material shader */
    for (uint32_t i = 0; i < handle->materials_count; ++i) {
        scene_internal_pbr_material *mat = handle->materials + i;
        gfx_shader_uniform_enable(mat->shader, "base_color_texture", GFX_TYPE_SAMPLER_2D,    &mat->base_color_tex_uniform);
        gfx_shader_uniform_enable(mat->shader, "has_color_texture",  GFX_TYPE_INTEGER_VEC_1, &mat->has_color_tex_uniform);
        gfx_shader_uniform_enable(mat->shader, "has_vertex_color",   GFX_TYPE_INTEGER_VEC_1, &mat->has_vertex_color_uniform);
    }

    /*
     * Shadow renderer — owns depth FBO + shader + light-space matrix.
     */
    shadow_renderer_init(&handle->shadow);

    /* Register shadow + BRDF LUT uniforms in every Lit material */
    for (uint32_t i = 0; i < handle->materials_count; ++i) {
        scene_internal_pbr_material *mat = handle->materials + i;
        gfx_shader_uniform_enable(mat->shader, "shadow_map",       GFX_TYPE_SAMPLER_2D,  &mat->shadow_map_uniform);
        gfx_shader_uniform_enable(mat->shader, "light_space",      GFX_TYPE_FLOAT_MAT_4, &mat->light_space_uniform);
        gfx_shader_uniform_enable(mat->shader, "brdf_lut",         GFX_TYPE_SAMPLER_2D,  &mat->brdf_lut_uniform);
        gfx_shader_uniform_enable(mat->shader, "prefiltered_env",  GFX_TYPE_SAMPLER_CUBE, &mat->prefiltered_env_uniform);
    }

    /*
     * Ground renderer — owns 20x20 quad + Lit shader instance.
     */
    ground_renderer_init(&handle->ground);

    /*
     * BRDF integration LUT — generated once via a GPU render pass.
     */
    handle->brdf_lut = brdf_lut_generate();

    /*
     * Loading of the meshes.
     */

    handle->meshes_count = model->meshes_count;
    handle->meshes = OS_MALLOC(sizeof(struct scene_internal_mesh) * model->meshes_count);

    for (uint32_t i = 0; i < handle->meshes_count; ++i) {
        scene_internal_mesh* mesh = handle->meshes + i;
        mdl_mesh* m_mesh = model->meshes + i;
        mesh->primitives_count = m_mesh->primitives_count;
        mesh->primitives = OS_MALLOC(sizeof(scene_internal_mesh_primitive) * m_mesh->primitives_count);
        for (uint32_t j = 0; j < m_mesh->primitives_count; ++j) {
            if(m_mesh->primitives[j].material_id < 0 || m_mesh->primitives[j].material_id >= handle->materials_count)
                m_mesh->primitives[j].material_id = 0;

            mesh->primitives[j] = scene_new_primitive(handle->materials[m_mesh->primitives[j].material_id].shader, handle->shadow.shader, m_mesh->primitives[j]);
        }
    }

    /*
     * Loading of the nodes.
     */

    handle->nodes_count = model->nodes_count;
    handle->nodes = OS_MALLOC(sizeof(struct scene_internal_node) * model->nodes_count);
    handle->active_camera_node = -1;
    int32_t root_nodes_count = 0, leaf_nodes_count = 0;
    for (uint32_t i = 0; i < handle->nodes_count; ++i) {

        scene_internal_node *node = handle->nodes + i;
        mdl_node *m_node = model->nodes + i;
        node->children_count = m_node->children_count;
        node->children_id = OS_MALLOC(sizeof(int32_t) * m_node->children_count);
        if (m_node->name != 0) {
            node->name = OS_MALLOC(sizeof(char) * (strlen(m_node->name) + 1));
            os_memcpy(node->name, m_node->name, sizeof(char) * (strlen(m_node->name) + 1));
        } else {
            node->name = 0;
        }
        node->parent_id = m_node->parent_id;
        node->local_id = i;

        gl_vec3 localPos;
        gl_vec3 localScale;
        gl_vec4 localRot;

        os_memcpy(localPos.data, m_node->local_pos, sizeof(float) * 3);
        os_memcpy(localRot.data, m_node->local_rot, sizeof(float) * 4);
        os_memcpy(localScale.data, m_node->local_scale, sizeof(float) * 3);

        gl_mat pos = gl_mat_translate(localPos);
        gl_mat rot = gl_mat_from_quaternion(localRot);
        gl_mat scale = gl_mat_scale(localScale);

        node->local_tr = gl_mat_mul(pos, gl_mat_mul(rot, scale));

        node->mesh_index = m_node->mesh_index;
        node->camera_index = m_node->camera_index;
        node->light_index = m_node->light_index;

        if(node->parent_id == -1)
            root_nodes_count++;

        if(node->children_count <= 0)
            leaf_nodes_count++;

        if(node->mesh_index != -1)
            handle->meshes[node->mesh_index].node_index = i;

        if(node->camera_index != -1)
            handle->cameras[node->camera_index].node_index = i;

        if(node->light_index != -1)
            handle->lights[node->light_index].node_index = i;

        if (node->camera_index != -1)
            handle->active_camera_node = i;

        os_memcpy(node->children_id, m_node->children_id, sizeof(int32_t) * m_node->children_count);
    }

    handle->root_nodes = OS_MALLOC(sizeof(void*) * root_nodes_count);
    handle->root_nodes_count = root_nodes_count;
    for(int32_t i=0, count=0; i<handle->nodes_count; ++i) {
        scene_internal_node *node = handle->nodes + i;
        if (node->parent_id == -1) {
            handle->root_nodes[count] = node;
            count++;
        }
    }

    /*
     * Update scene graph.
     */

    for(int32_t i=0; i<handle->nodes_count; ++i) {
        scene_internal_node* cur_node = handle->nodes + i;
        gl_mat world_tr = cur_node->local_tr;
        while (cur_node->parent_id != -1) {
            cur_node = handle->nodes + cur_node->parent_id;
            world_tr = gl_mat_mul(cur_node->local_tr, world_tr);
        }
        cur_node = handle->nodes + i;
        cur_node->world_tr = world_tr;
    }

    /*
     * Highlight shader — inverted-hull outline for selected node.
     */
    handle->selected_node_id = -1;
    {
        void* hl_vs = device_file_read_text("./Shaders/Highlight.vs");
        void* hl_fs = device_file_read_text("./Shaders/Highlight.fs");
        handle->highlight_shader = gfx_shader_create("Highlight");
        gfx_shader_add_vs(handle->highlight_shader, hl_vs);
        gfx_shader_add_fs(handle->highlight_shader, hl_fs);
        gfx_shader_submit(handle->highlight_shader);
        OS_FREE(hl_vs);
        OS_FREE(hl_fs);

        gfx_shader_uniform_enable(handle->highlight_shader, "model",         GFX_TYPE_FLOAT_MAT_4, &handle->hl_model_u);
        gfx_shader_uniform_enable(handle->highlight_shader, "view",          GFX_TYPE_FLOAT_MAT_4, &handle->hl_view_u);
        gfx_shader_uniform_enable(handle->highlight_shader, "projection",    GFX_TYPE_FLOAT_MAT_4, &handle->hl_proj_u);
        gfx_shader_uniform_enable(handle->highlight_shader, "outline_scale", GFX_TYPE_FLOAT_VEC_1, &handle->hl_scale_u);
        gfx_shader_uniform_enable(handle->highlight_shader, "outline_color", GFX_TYPE_FLOAT_VEC_4, &handle->hl_color_u);

        /* Build a highlight pipeline for each mesh primitive (pos + normal only). */
        for (uint32_t mi = 0; mi < handle->meshes_count; ++mi) {
            scene_internal_mesh *mesh = handle->meshes + mi;
            mdl_mesh *m_mesh = model->meshes + mi;
            for (uint32_t pi = 0; pi < (uint32_t)mesh->primitives_count; ++pi) {
                scene_internal_mesh_primitive *prim = mesh->primitives + pi;
                prim->highlight_pipeline = gfx_pipeline_create(handle->highlight_shader);
                for (int32_t ai = 0; ai < m_mesh->primitives[pi].attributes_count; ++ai) {
                    mdl_attribute *attr = m_mesh->primitives[pi].attributes + ai;
                    if (attr->type == MDL_VERTEX_ATTRIBUTE_POSITION) {
                        gfx_pipeline_attr_enable(prim->highlight_pipeline, ATTR_POSITION_NAME,
                            prim->buffer_handle, attr->count, attr->offset,
                            m_mesh->primitives[pi].vertex_stride);
                    } else if (attr->type == MDL_VERTEX_ATTRIBUTE_NORMAL) {
                        gfx_pipeline_attr_enable(prim->highlight_pipeline, ATTR_NORMAL_NAME,
                            prim->buffer_handle, attr->count, attr->offset,
                            m_mesh->primitives[pi].vertex_stride);
                    }
                }
                gfx_pipeline_index_enable(prim->highlight_pipeline, prim->index_handle);
                gfx_pipeline_submit(prim->highlight_pipeline);
            }
        }
    }

    return handle;
}

void scene_shadow_pass(scene_handle handle) {
    shadow_renderer* sr = &handle->shadow;
    static float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    gfx_begin_pass(sr->fbo,
                   GFX_PASS_OPTION_DEPTH_TEST | GFX_PASS_OPTION_CULL_BACK,
                   GFX_PASS_ACTION_CLEAR_DEPTH, black);
    gfx_viewport_set(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    for (int32_t i = 0; i < handle->meshes_count; ++i) {
        scene_internal_mesh *mesh      = handle->meshes + i;
        scene_internal_node  mesh_node = handle->nodes[mesh->node_index];
        for (int32_t j = 0; j < mesh->primitives_count; ++j) {
            scene_internal_mesh_primitive *prim = mesh->primitives + j;
            gfx_pipeline_bind(prim->shadow_pipeline);
            gfx_shader_uniform_set(sr->shader, sr->model_uniform, mesh_node.world_tr.data);
            gfx_shader_uniform_set(sr->shader, sr->ls_uniform,    sr->light_space.data);
            gfx_draw_id(prim->draw_type, prim->indices_count);
        }
    }

    gfx_end_pass();
}

void scene_draw(scene_handle handle) {

    if (handle->active_camera_node == -1) {
        return;
    }

    scene_internal_node *camera_node = handle->nodes + (handle->active_camera_node);
    scene_camera camera;
    scene_node node;
    scene_camera_get_at(handle, camera_node->camera_index, &node, &camera);
    scene_draw_with_camera(handle, camera.projection, camera_node->world_tr.data, true, false);
}

void scene_draw_with_camera(scene_handle handle, float projection[16], float tr[16], bool draw_skybox, bool wireframe){

    gl_mat world_tr = gl_mat_new_array(tr);
    gl_mat view = gl_mat_inverse(world_tr);
    gl_mat view_no_tr = gl_mat_remove_translation(view);
    gl_vec3 view_pos = gl_mat_get_translation(world_tr);
    int32_t texture_unit   = 0;   /* skybox cubemap      */
    int32_t shadow_unit    = 1;   /* shadow depth        */
    int32_t brdf_unit      = 2;   /* BRDF LUT            */
    int32_t prefilter_unit = 3;   /* prefiltered env IBL */
    float exposure = handle->skybox_enabled ? skybox_get_exposure(handle->skybox) : 1.0f;
    if(handle->skybox_enabled) {
        skybox_bind(handle->skybox);  /* binds cubemap to unit 0 */
    }
    gfx_texture_bind(handle->shadow.depth_tex, shadow_unit);
    gfx_texture_bind(handle->brdf_lut,         brdf_unit);
    if (handle->prefiltered_env_id) {
        prefilter_env_bind(handle->prefiltered_env_id, prefilter_unit);
    }

    if(handle->skybox_enabled && handle->skybox_render && draw_skybox){
        skybox_render(handle->skybox, projection, view_no_tr.data);
    }

    gfx_wireframe_enable(wireframe);

    for (int32_t i = 0; i < handle->meshes_count; ++i) {
        scene_internal_mesh *mesh = handle->meshes + i;
        for (int32_t j = 0; j < mesh->primitives_count; ++j) {

            scene_internal_mesh_primitive *primitive = mesh->primitives + j;
            gfx_pipeline_bind(primitive->pipeline_handle);
            scene_internal_pbr_material *mat = handle->materials + primitive->material_id;

            if (primitive->material_id != -1 && mat->valid) {

                scene_internal_node mesh_node = handle->nodes[mesh->node_index];

                gfx_shader_uniform_set(mat->shader, mat->view_position_uniform, view_pos.data);
                gfx_shader_uniform_set(mat->shader, mat->model_uniform, mesh_node.world_tr.data);
                gfx_shader_uniform_set(mat->shader, mat->view_uniform, view.data);
                gfx_shader_uniform_set(mat->shader, mat->projection_uniform, projection);
                gfx_shader_uniform_set(mat->shader, mat->color_uniform, mat->color_factor);
                gfx_shader_uniform_set(mat->shader, mat->rougness_uniform, &mat->roughness_factor);
                gfx_shader_uniform_set(mat->shader, mat->metallic_uniform, &mat->metallic_factor);
                gfx_shader_uniform_set(mat->shader, mat->skybox_uniform, &texture_unit);
                gfx_shader_uniform_set(mat->shader, mat->exposure_uniform, &exposure);
                gfx_shader_uniform_set(mat->shader, mat->shadow_map_uniform,  &shadow_unit);
                gfx_shader_uniform_set(mat->shader, mat->light_space_uniform, handle->shadow.light_space.data);
                gfx_shader_uniform_set(mat->shader, mat->brdf_lut_uniform,           &brdf_unit);
                gfx_shader_uniform_set(mat->shader, mat->prefiltered_env_uniform, &prefilter_unit);

                /* Base color texture (unit 4) */
                int32_t base_color_unit = 4;
                int32_t has_color_tex   = 0;
                if (mat->color_texture_id >= 0 &&
                    mat->color_texture_id < (int32_t)handle->textures_count &&
                    handle->textures[mat->color_texture_id].texture_handle != 0) {
                    gfx_texture_bind(handle->textures[mat->color_texture_id].texture_handle,
                                     base_color_unit);
                    has_color_tex = 1;
                }
                int32_t has_vtx_col = primitive->has_vertex_color ? 1 : 0;
                gfx_shader_uniform_set(mat->shader, mat->base_color_tex_uniform,    &base_color_unit);
                gfx_shader_uniform_set(mat->shader, mat->has_color_tex_uniform,     &has_color_tex);
                gfx_shader_uniform_set(mat->shader, mat->has_vertex_color_uniform,  &has_vtx_col);

            }

            gfx_draw_id(primitive->draw_type, primitive->indices_count);
        }
    }

    /* Ground plane */
    if (handle->plane_render) {
        ground_renderer_render(&handle->ground,
                               projection, view.data, view_pos.data,
                               texture_unit, exposure,
                               shadow_unit, handle->shadow.light_space.data,
                               brdf_unit, prefilter_unit);
    }

    gfx_wireframe_enable(false);

    /* Highlight pass: inverted-hull outline for selected node */
    if (handle->selected_node_id >= 0) {
        scene_internal_node *sel = &handle->nodes[handle->selected_node_id];
        if (sel->mesh_index >= 0 && sel->mesh_index < (int32_t)handle->meshes_count) {
            scene_internal_mesh *mesh = &handle->meshes[sel->mesh_index];
            float outline_scale = 0.025f;
            float outline_color[4] = {1.0f, 0.55f, 0.05f, 1.0f};
            glCullFace(GL_FRONT);
            for (int32_t j = 0; j < mesh->primitives_count; ++j) {
                scene_internal_mesh_primitive *prim = &mesh->primitives[j];
                gfx_pipeline_bind(prim->highlight_pipeline);
                gfx_shader_uniform_set(handle->highlight_shader, handle->hl_model_u, sel->world_tr.data);
                gfx_shader_uniform_set(handle->highlight_shader, handle->hl_view_u,  view.data);
                gfx_shader_uniform_set(handle->highlight_shader, handle->hl_proj_u,  projection);
                gfx_shader_uniform_set(handle->highlight_shader, handle->hl_scale_u, &outline_scale);
                gfx_shader_uniform_set(handle->highlight_shader, handle->hl_color_u, outline_color);
                gfx_draw_id(prim->draw_type, prim->indices_count);
            }
            glCullFace(GL_BACK);
        }
    }
}

bool scene_get_skybox_render(scene_handle handle) {
    return handle->skybox_render;
}

void scene_set_skybox_render(scene_handle handle, bool render) {
    handle->skybox_render = render;
}

void scene_set_skybox_path(scene_handle handle, const char* path) {
    float exposure = scene_get_skybox_exposure(handle);

    if (handle->skybox_enabled) {
        skybox_destroy(handle->skybox);
        handle->skybox = 0;
        handle->skybox_enabled = false;
    }
    if (handle->prefiltered_env_id) {
        prefilter_env_destroy(handle->prefiltered_env_id);
        handle->prefiltered_env_id = 0;
    }

    if (path != 0 && path[0] != '\0') {
        handle->skybox = skybox_load(path);
        skybox_set_exposure(handle->skybox, gl_clamp(exposure, 0.1f, 5.0f));
        handle->skybox_enabled = true;
        handle->prefiltered_env_id = prefilter_env_generate(skybox_get_cubemap_id(handle->skybox));
    }
}

float scene_get_skybox_exposure(scene_handle handle) {
    if (!handle->skybox_enabled) {
        return 1.0f;
    }
    return skybox_get_exposure(handle->skybox);
}

void scene_set_skybox_exposure(scene_handle handle, float exposure) {
    if (!handle->skybox_enabled) {
        return;
    }
    skybox_set_exposure(handle->skybox, gl_clamp(exposure, 0.1f, 5.0f));
}

bool scene_get_plane_render(scene_handle handle) {
    return handle->plane_render;
}

void scene_set_plane_render(scene_handle handle, bool render) {
    handle->plane_render = render;
}

void scene_delete(scene_handle handle) {
    for(int32_t i=0; i<handle->textures_count; ++i)
        gfx_texture_destroy(handle->textures[i].texture_handle);
    if(handle->textures != 0)
        OS_FREE(handle->textures);

    if(handle->cameras != 0)
        OS_FREE(handle->cameras);
    if(handle->lights != 0)
        OS_FREE(handle->lights);
    for(int32_t i=0; i<handle->meshes_count; ++i)
    {
        scene_internal_mesh * mesh = handle->meshes + i;
        for(int32_t j=0; j<mesh->primitives_count; ++j)
        {
            scene_internal_mesh_primitive* primitive = mesh->primitives + j;
            gfx_buffer_destroy(primitive->buffer_handle);
            gfx_buffer_destroy(primitive->index_handle);
            gfx_pipeline_destroy(primitive->pipeline_handle);
            gfx_pipeline_destroy(primitive->shadow_pipeline);
            gfx_pipeline_destroy(primitive->highlight_pipeline);
        }
        OS_FREE(mesh->primitives);
    }

    if(handle->materials != 0) {
        for (int32_t i = 0; i < handle->materials_count; ++i) {
            gfx_shader_destroy(handle->materials[i].shader);
        }
        OS_FREE(handle->materials);
    }

    OS_FREE(handle->meshes);

    for(int32_t i=0; i<handle->nodes_count; ++i)
    {
        scene_internal_node * node = handle->nodes + i;
        OS_FREE(node->children_id);
        OS_FREE(node->name);
    }

    if(handle->skybox_enabled)
        skybox_destroy(handle->skybox);

    shadow_renderer_destroy(&handle->shadow);
    ground_renderer_destroy(&handle->ground);
    gfx_texture_destroy(handle->brdf_lut);
    if (handle->prefiltered_env_id)
        prefilter_env_destroy(handle->prefiltered_env_id);
    if (handle->highlight_shader)
        gfx_shader_destroy(handle->highlight_shader);

    OS_FREE(handle->root_nodes);
    OS_FREE(handle->nodes);
    OS_FREE(handle);
}

bool scene_node_get(scene_handle handle, const char *name, scene_node * out_node) {
    os_memset(out_node, 0, sizeof(scene_node));
    for(int32_t i=0; i<handle->nodes_count; ++i) {
        if (handle->nodes[i].name == 0) continue;
        if (strcmp(handle->nodes[i].name, name) == 0) {
            scene_node_get_at(handle, i, out_node);
            return true;
        }
    }
    return false;
}

void scene_node_set_camera(scene_handle handle, scene_node* node, scene_camera* camera_data) {
    scene_internal_node *node_int = (scene_internal_node *) node->internal;
    struct scene_internal_camera *camera = handle->cameras + node_int->camera_index;

    camera->ortographic = !camera_data->perspective;
    camera->zfar = camera_data->zfar;
    camera->znear = camera_data->znear;
    camera->fov = camera_data->fov;
    camera->ar = camera_data->ar;

    if (camera_data->perspective) {
        camera->projection = gl_mat_perspective(camera->fov, camera->ar, camera->znear,
                                                camera->zfar);
    } else {
        int32_t height = camera->fov;
        int32_t width = camera->fov * camera->ar;
        camera->projection = gl_mat_ortographic(-height , height, -width,
                                                width, camera->znear,
                                                camera->zfar);
    }
}

void scene_node_count(scene_handle handle, int32_t* count){
    *count = handle->nodes_count;
}

void scene_node_children_count(scene_handle handle, scene_node* node, int32_t* count){
    scene_internal_node *node_int = (scene_internal_node *) node->internal;
    *count = node_int->children_count;
}

void scene_camera_count(scene_handle handle, int32_t* count){
    *count = handle->cameras_count;
}

void scene_mesh_count(scene_handle handle, int32_t* count){
    *count = handle->meshes_count;
}

void scene_node_get_at(scene_handle handle, int32_t i, scene_node *out_node){
    scene_internal_node node = handle->nodes[i];
    out_node->name = node.name;
    out_node->internal = handle->nodes + i;
}

void scene_node_get_world_tr(scene_handle handle, scene_node* node, float* tr){
    scene_internal_node *node_int = (scene_internal_node *) node->internal;
    os_memcpy(tr, node_int->world_tr.data, sizeof(gl_mat));
}

void scene_node_set_world_tr(scene_handle handle, scene_node* node, float* tr){
    scene_internal_node *node_int = (scene_internal_node *) node->internal;
    scene_internal_node *cur_node = node_int;

    if (cur_node->parent_id >= 0) {
        cur_node = node_int;

        gl_mat local_tr = GL_MAT_IDENTITY;
        while (cur_node->parent_id != -1) {
            local_tr = gl_mat_mul((handle->nodes + cur_node->parent_id)->local_tr, local_tr);
            cur_node = handle->nodes + cur_node->parent_id;
        }
        node_int->local_tr = gl_mat_mul(gl_mat_inverse(local_tr), gl_mat_new_array(tr));
    } else {
        os_memcpy(node_int->world_tr.data, tr, sizeof(float) * 16);
    }

    for(int32_t i=0; i<handle->nodes_count; ++i) {
        cur_node = handle->nodes + i;

        while (cur_node->parent_id != -1 && cur_node != node_int) {
            cur_node = handle->nodes + cur_node->parent_id;
        }

        if (cur_node != node_int)
            continue;

        cur_node = handle->nodes + i;
        gl_mat world_tr = cur_node->local_tr;
        while (cur_node->parent_id != -1) {
            cur_node = handle->nodes + cur_node->parent_id;
            world_tr = gl_mat_mul(cur_node->local_tr, world_tr);
        }
        cur_node = handle->nodes + i;
        cur_node->world_tr = world_tr;
    }
}

void scene_node_get_local_tr(scene_handle handle, scene_node* node, float* tr){
    scene_internal_node *node_int = (scene_internal_node *) node->internal;
    os_memcpy(tr, node_int->local_tr.data, sizeof(gl_mat));
}

void scene_node_set_local_tr(scene_handle handle, scene_node* node, float* tr){
    scene_internal_node *node_int = (scene_internal_node *) node->internal;
    os_memcpy(node_int->local_tr.data, tr, sizeof(float) * 16);

    scene_internal_node *cur_node = node_int;
    for(int32_t i=0; i<handle->nodes_count; ++i) {
        cur_node = handle->nodes + i;

        while (cur_node->parent_id != -1 && cur_node != node_int) {
            cur_node = handle->nodes + cur_node->parent_id;
        }

        if (cur_node != node_int)
            continue;

        cur_node = handle->nodes + i;
        gl_mat world_tr = cur_node->local_tr;
        while (cur_node->parent_id != -1) {
            cur_node = handle->nodes + cur_node->parent_id;
            world_tr = gl_mat_mul(cur_node->local_tr, world_tr);
        }
        cur_node = handle->nodes + i;
        cur_node->world_tr = world_tr;
    }
}

void scene_node_root_count(scene_handle handle, int32_t *count) {
    *count = handle->root_nodes_count;
}

void scene_node_root_get_at(scene_handle handle, int32_t index, struct scene_node *node) {
    scene_node_get_at(handle, (handle->root_nodes[index])->local_id, node);
}

void scene_node_children_get_at(scene_handle handle, int32_t index, struct scene_node *parent_node, scene_node *node) {
    scene_internal_node *node_int = (scene_internal_node *) parent_node->internal;
    scene_node_get_at(handle, handle->nodes[*(node_int->children_id + index)].local_id, node);
}

void scene_camera_get_at(scene_handle handle, int32_t index, struct scene_node *node, struct scene_camera *camera_data) {
    scene_internal_camera *int_camera = handle->cameras + index;
    scene_node_get_at(handle, int_camera->node_index, node);
    camera_data->perspective = !int_camera->ortographic;
    camera_data->zfar = int_camera->zfar;
    camera_data->znear = int_camera->znear;
    camera_data->ar = int_camera->ar;
    camera_data->fov = int_camera->fov;
    os_memcpy(camera_data->projection, int_camera->projection.data, sizeof(gl_mat));
}

void scene_camera_set(scene_handle handle, scene_node* node, scene_camera* camera_data) {
    scene_node_set_camera(handle, node, camera_data);
}

/* ---- Selection ---- */

void scene_set_selected_node(scene_handle handle, scene_node* node) {
    if (node == NULL) {
        handle->selected_node_id = -1;
    } else {
        scene_internal_node *n = (scene_internal_node *)node->internal;
        handle->selected_node_id = n->local_id;
    }
}

bool scene_get_selected_node(scene_handle handle, scene_node* node_out) {
    if (handle->selected_node_id < 0) return false;
    scene_node_get_at(handle, handle->selected_node_id, node_out);
    return true;
}

/* ---- Material inspection ---- */

int32_t scene_node_get_material_count(scene_handle handle, scene_node* node) {
    scene_internal_node *n = (scene_internal_node *)node->internal;
    if (n->mesh_index < 0 || n->mesh_index >= (int32_t)handle->meshes_count) return 0;
    return handle->meshes[n->mesh_index].primitives_count;
}

void scene_node_get_material(scene_handle handle, scene_node* node, int32_t mat_idx,
                             float color[4], float* metallic, float* roughness) {
    scene_internal_node *n = (scene_internal_node *)node->internal;
    if (n->mesh_index < 0) return;
    scene_internal_mesh *mesh = &handle->meshes[n->mesh_index];
    if (mat_idx < 0 || mat_idx >= mesh->primitives_count) return;
    int32_t mid = mesh->primitives[mat_idx].material_id;
    if (mid < 0 || mid >= (int32_t)handle->materials_count) return;
    scene_internal_pbr_material *mat = &handle->materials[mid];
    if (color)     os_memcpy(color, mat->color_factor, sizeof(float) * 4);
    if (metallic)  *metallic  = mat->metallic_factor;
    if (roughness) *roughness = mat->roughness_factor;
}

void scene_node_set_material(scene_handle handle, scene_node* node, int32_t mat_idx,
                             float color[4], float metallic, float roughness) {
    scene_internal_node *n = (scene_internal_node *)node->internal;
    if (n->mesh_index < 0) return;
    scene_internal_mesh *mesh = &handle->meshes[n->mesh_index];
    if (mat_idx < 0 || mat_idx >= mesh->primitives_count) return;
    int32_t mid = mesh->primitives[mat_idx].material_id;
    if (mid < 0 || mid >= (int32_t)handle->materials_count) return;
    scene_internal_pbr_material *mat = &handle->materials[mid];
    if (color) os_memcpy(mat->color_factor, color, sizeof(float) * 4);
    mat->metallic_factor  = metallic;
    mat->roughness_factor = roughness;
}
