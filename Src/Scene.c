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

#include <string.h>
#include <stdio.h>

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
    int32_t color_uniform;

    gfx_shader_handle shader;
} scene_internal_pbr_material;

typedef struct scene_internal_mesh_primitive{
    int32_t indices_count;
    int32_t material_id;

    gfx_pipeline_handle pipeline_handle;
    gfx_buffer_handle buffer_handle;
    gfx_buffer_handle index_handle;

    gfx_draw_type draw_type;
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
} scene_internal_data;

scene_internal_mesh_primitive scene_new_primitive(gfx_shader_handle shader, mdl_primitive primitive) {
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

    result.indices_count = primitive.indices_count;
    result.index_handle = index_handle;
    result.material_id = primitive.material_id;
    return result;
}

scene_handle scene_new(scene_desc const* desc) {
    mdl_data* model = desc->model;

    //create scene structure
    scene_handle handle = OS_MALLOC(sizeof(struct scene_internal_data));
    os_memset(handle, 0, sizeof(scene_internal_data));


    handle->skybox_enabled = desc->skybox.path != 0;
    handle->skybox_render = desc->skybox.render;
    if(desc->skybox.path != 0) {
        handle->skybox = skybox_load(desc->skybox.path);
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
            camera->projection = gl_mat_perspective(camera->fov * 100, camera->ar, camera->znear,
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
        gfx_shader_uniform_enable(mat->shader, "base_color", GFX_TYPE_FLOAT_VEC_3, &mat->color_uniform);

    }
    OS_FREE(vs);
    OS_FREE(fs);

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
            if(m_mesh->primitives[j].material_id == -1)
                m_mesh->primitives[j].material_id = 0;

            mesh->primitives[j] = scene_new_primitive(handle->materials[m_mesh->primitives[j].material_id].shader, m_mesh->primitives[j]);
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
        node->name = OS_MALLOC(sizeof(char) * (strlen(m_node->name) + 1));
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

        os_memcpy(node->name, m_node->name, sizeof(char) * (strlen(m_node->name) + 1));
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
    return handle;
}

void scene_draw(scene_handle handle) {

    if (handle->active_camera_node == -1) {
        return;
    }

    scene_internal_node *camera_node = handle->nodes + (handle->active_camera_node);
    scene_camera camera;
    scene_node node;
    scene_camera_get_at(handle, camera_node->camera_index, &node, &camera);
    scene_draw_with_camera(handle, camera.projection, camera_node->world_tr.data, true);
}

void scene_draw_with_camera(scene_handle handle, float projection[16], float tr[16], bool draw_skybox){

    gl_mat world_tr = gl_mat_new_array(tr);
    gl_mat view = gl_mat_inverse(world_tr);
    gl_mat view_no_tr = gl_mat_remove_translation(view);
    gl_vec3 view_pos = gl_mat_get_translation(world_tr);

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

            }

            gfx_draw_id(primitive->draw_type, primitive->indices_count);
        }
    }

    if(handle->skybox_render && draw_skybox){
        skybox_render(handle->skybox, projection, view_no_tr.data);
    }
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
    os_memcpy(node_int->local_tr.data, tr, sizeof(gl_mat));

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
    }}

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

