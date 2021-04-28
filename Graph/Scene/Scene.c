/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Scene.h"

#include <Os/Allocator.h>
#include <Graphics/Graphics.h>
#include <Shaders/Lit.h>
#include <Model/Model.h>
#include <Math/GlMath.h>
#include <Device/Device.h>
#include <string.h>
#include <Draw/Draw.h>

typedef struct scene_node {
    char* name;
    gl_mat local_tr;

    int32_t parent_id;

    int32_t * children_id;
    int32_t children_count;
    int32_t mesh_index;
    int32_t camera_index;
    int32_t light_index;
} scene_node;

typedef struct scene_texture{
    gfx_texture_handle texture_handle;
} scene_texture;

typedef struct scene_material{
    mdl_material base;

    gfx_uniform model_uniform;
    gfx_uniform projection_uniform;
    gfx_uniform view_uniform;

    gfx_uniform ambient_color;
    gfx_uniform ambient_intensity;

    gfx_uniform sun_color;
    gfx_uniform sun_direction;
    gfx_uniform sun_intensity;

    gfx_uniform color_texture;
    gfx_uniform color_factor_uniform;



} scene_material;

typedef struct scene_mesh_primitive{
    int32_t indices_count;
    int32_t material_id;

    //todo: introduce one global buffer(pros: faster cons: removing nodes is not trivial/ impossible in case of static buffer?)
    gfx_pipeline_handle pipeline_handle;
    gfx_buffer_handle buffer_handle;
    gfx_buffer_handle index_handle;


    gl_vec3 min, max;
} scene_mesh_primitive;

typedef struct scene_mesh{
    gl_mat world_tr;
    uint32_t primitives_count;
    scene_mesh_primitive* primitives;
} scene_mesh;

typedef struct scene_light{

} scene_light;

typedef struct scene_data{
    uint32_t root_nodes_count;
    scene_node* root_nodes;

    uint32_t meshes_count;
    scene_mesh* meshes;

    uint32_t nodes_count;
    scene_node* nodes;

    uint32_t materials_count;
    scene_material* materials;

    uint32_t textures_count;
    scene_texture* textures;

    //todo: shader array
    gfx_shader_handle shader;
    bool wireframe;
    bool bounding_box;
    dw_handle dw;

    scene_lighting_settings lighting_settings;
    scene_sun_settings sun_settings;

} scene_data;


typedef struct controller_camera_data{
    gl_vec3 pos;
    gl_vec3 rot_euler;
    gl_mat projection;
    gl_mat view;
} controller_camera_data;


controller_camera_data controller_data;


scene_mesh_primitive scene_new_primitive(gfx_shader_handle shader, mdl_primitive primitive) {
    scene_mesh_primitive result = {0};
    gfx_buffer_desc buffer_desc = {
            .data = primitive.vertices,
            .size = primitive.vertex_stride * primitive.vertices_count,
            .type = GFX_BUFFER_VERTEX,
            .update_mode = GFX_BUFFER_UPDATE_STATIC_DRAW,
    };

    result.buffer_handle = gfx_buffer_create(&buffer_desc);


    //create attributes
    gfx_pipeline_attr active_attributes[MAXIMUM_PIPELINE_ATTRIBUTES];
    int32_t position_attribute_id = -1;
    os_memset(active_attributes, 0, sizeof(gfx_pipeline_attr) * MAXIMUM_PIPELINE_ATTRIBUTES);
    for (int32_t i = 0; i < primitive.attributes_count; ++i) {
        gfx_pipeline_attr *attr = 0;
        switch (primitive.attributes[i].type) {

            case MDL_VERTEX_ATTRIBUTE_INVALID: attr = 0;
                break;
            case MDL_VERTEX_ATTRIBUTE_POSITION:attr = active_attributes + ATTR_POSITION_LOCATION;
                position_attribute_id = i;
                break;
            case MDL_VERTEX_ATTRIBUTE_UV: attr = active_attributes + ATTR_UV_LOCATION;
                break;
            case MDL_VERTEX_ATTRIBUTE_NORMAL: attr = active_attributes + ATTR_NORMAL_LOCATION;
                break;
            case MDL_VERTEX_ATTRIBUTE_COLOR: attr = active_attributes + ATTR_COLOR_LOCATION;
                break;
            case MDL_VERTEX_ATTRIBUTE_TANGENT: attr = active_attributes + ATTR_TANGENT_LOCATION;
                break;
            case MDL_VERTEX_ATTRIBUTE_WEIGHTS: attr = active_attributes + ATTR_WEIGHTS_LOCATION;
                break;
            case MDL_VERTEX_ATTRIBUTE_JOINTS: attr = active_attributes + ATTR_JOINTS_LOCATION;
                break;
        }
        if (attr != 0) {
            attr->buffer = result.buffer_handle;
            attr->offset = primitive.attributes[i].offset;
            attr->stride = primitive.vertex_stride;
            attr->enabled = true;
        }
    }

    gfx_buffer_desc index_buffer_desc = {
            .data = primitive.indices,
            .size = primitive.indices_count * sizeof(uint32_t),
            .type = GFX_BUFFER_INDEX,
            .update_mode = GFX_BUFFER_UPDATE_STATIC_DRAW
    };
    gfx_buffer_handle index_handle = gfx_buffer_create(&index_buffer_desc);
    gfx_pipeline_desc pip_desc = {
            .shader = shader,
            .index_buffer = index_handle,
    };

    os_memcpy(pip_desc.attrs, active_attributes, sizeof(gfx_pipeline_attr) * MAXIMUM_PIPELINE_ATTRIBUTES);

    if (position_attribute_id != -1) {
#define GL_MAX 100000
        result.min = gl_vec3_new(GL_MAX, GL_MAX, GL_MAX);
        result.max = gl_vec3_new(-GL_MAX, -GL_MAX, -GL_MAX);
        //calculate min and max
        for (int32_t i = 0; i < primitive.vertices_count; ++i) {
            void *v = ((char *) primitive.vertices + active_attributes[position_attribute_id].stride * i) +
                      active_attributes[position_attribute_id].offset;
            result.min.x = gl_min(result.min.x, *((float *) v));
            result.min.y = gl_min(result.min.y, *((float *) v + 1));
            result.min.z = gl_min(result.min.z, *((float *) v + 2));

            result.max.x = gl_max(result.max.x, *((float *) v));
            result.max.y = gl_max(result.max.y, *((float *) v + 1));
            result.max.z = gl_max(result.max.z, *((float *) v + 2));
        }
    }

    result.pipeline_handle = gfx_pipeline_create(&pip_desc);
    result.indices_count = primitive.indices_count;
    result.index_handle = index_handle;
    result.material_id = primitive.material_id;
    return result;
}

scene_handle scene_new(void* model_ptr, struct scene_sun_settings sun_settings, struct scene_lighting_settings lighting_settings) {
    os_memset(&controller_data, 0, sizeof(controller_camera_data));

    int32_t width, height;
    device_window_dimensions_get(&width, &height);

    controller_data.pos = gl_vec3_new(0,1,-10);
    controller_data.projection = gl_mat_perspective(75, (float)width / height, 0.01, 100);

    mdl_data* model = model_ptr;
    gfx_shader_handle sh_handle = gfx_shader_create(&lit_shader_desc);

    scene_handle handle = OS_MALLOC(sizeof(struct scene_data));
    os_memset(handle, 0, sizeof(scene_data));

    dw_desc desc = {};

    handle->shader = sh_handle;
    handle->dw = dw_new(&desc);
    handle->sun_settings = sun_settings;
    handle->lighting_settings = lighting_settings;

    /*
     * Loading of the meshes.
     */

    handle->meshes_count = model->meshes_count;
    handle->meshes = OS_MALLOC(sizeof(struct scene_mesh) * model->meshes_count);

    for (uint32_t i = 0; i < handle->meshes_count; ++i) {
        scene_mesh* mesh = handle->meshes + i;
        mdl_mesh* m_mesh = model->meshes + i;
        mesh->primitives_count = m_mesh->primitives_count;
        mesh->primitives = OS_MALLOC(sizeof(scene_mesh_primitive) * m_mesh->primitives_count);
        for (uint32_t j = 0; j < m_mesh->primitives_count; ++j) {
            mesh->primitives[j] = scene_new_primitive(sh_handle, m_mesh->primitives[j]);
            gl_vec3 center = gl_vec3_div(gl_vec3_add(mesh->primitives[j].max, mesh->primitives[j].min), gl_vec3_new_scalar(2));
            gl_vec3 size = gl_vec3_sub(mesh->primitives[j].max, mesh->primitives[j].min);
            dw_cube(handle->dw, center, gl_vec3_new(1, 0, 0), size);
        }
    }

    /*
     * Loading of the nodes.
     */


    handle->nodes_count = model->nodes_count;
    handle->nodes = OS_MALLOC(sizeof(struct scene_node) * model->nodes_count);

    for (uint32_t i = 0; i < handle->nodes_count; ++i) {
        scene_node *node = handle->nodes + i;
        mdl_node *m_node = model->nodes + i;
        node->children_count = m_node->children_count;
        node->children_id = OS_MALLOC(sizeof(int32_t) * m_node->children_count);
        node->name = OS_MALLOC(sizeof(char) * (strlen(m_node->name)+1));
        node->parent_id = m_node->parent_id;
        node->local_tr = gl_mat_new_array(m_node->tr_local);
        node->mesh_index = m_node->mesh_index;
        node->camera_index = m_node->camera_index;
        node->light_index = m_node->mesh_index;
        if(node->mesh_index != -1)  handle->meshes[node->mesh_index].world_tr = gl_mat_new_array(m_node->tr_local);
        os_memcpy(node->name, m_node->name, sizeof(char) * (strlen(m_node->name)+1));
        os_memcpy(node->children_id, m_node->children_id, sizeof(int32_t) * m_node->children_count);
    }


    /*
     * Loading of the materials.
     */
    handle->materials_count = model->materials_count;
    handle->materials = OS_MALLOC(sizeof(struct scene_material) * model->materials_count);

    for (uint32_t i = 0; i < model->materials_count; ++i) {
        struct scene_material *mat = handle->materials + i;
        struct mdl_material *m_mat = model->materials + i;
        os_memcpy(&(mat->base), m_mat, sizeof(struct mdl_material));

        mat->model_uniform = gfx_uniform_register(handle->shader, "model", GFX_MAT4);
        mat->projection_uniform = gfx_uniform_register(handle->shader, "projection", GFX_MAT4);
        mat->view_uniform = gfx_uniform_register(handle->shader, "view", GFX_MAT4);

        mat->color_factor_uniform = gfx_uniform_register(handle->shader, "colorFactor", GFX_FLOAT4);

        mat->ambient_color = gfx_uniform_register(handle->shader, "ambientColor", GFX_FLOAT3);
        mat->ambient_intensity = gfx_uniform_register(handle->shader, "ambientIntensity", GFX_FLOAT1);

        mat->sun_color = gfx_uniform_register(handle->shader, "sunColor", GFX_FLOAT3);
        mat->sun_direction = gfx_uniform_register(handle->shader, "sunDirection", GFX_FLOAT3);
        mat->sun_intensity = gfx_uniform_register(handle->shader, "sunIntensity", GFX_FLOAT3);
    }

    /*
     * Loading of the textures.
     */
    handle->textures_count = model->textures_count;
    handle->textures = OS_MALLOC(sizeof(struct scene_texture) * model->textures_count);

    for (uint32_t i = 0; i < model->textures_count; ++i) {
        struct scene_texture *tex = handle->textures + i;
        struct mdl_texture *m_tex = model->textures + i;
        if (m_tex->valid) {
            gfx_texture_type tex_type = gfx_texture_color_type_from_channels(m_tex->channels, true);
            gfx_texture_desc desc_tex = {
                    .data = m_tex->buffer,
                    .width = m_tex->width,
                    .height = m_tex->height,
                    .wrap = GFX_TEXTURE_WRAP_REPEAT,
                    .mipmaps = true,
                    .filter = GFX_TEXTURE_FILTER_NEAREST,
                    .type = tex_type
            };
            tex->texture_handle = gfx_texture_create(&desc_tex);
        } else {
            tex->texture_handle = 0;
        }
    }

    return handle;
}
void controller_camera_fp_update(controller_camera_data* handle, float dt) {

    gl_vec3 up = GL_VEC3_UP;
    gl_vec3 fwd = GL_VEC3_FWD;
    gl_vec3 delta = GL_VEC3_ZERO;

    device_joystick joystick = device_joystick_get();
    delta.x = joystick.horiz.value;
    delta.z = joystick.vert.value;

    //rotation
    handle->rot_euler.x = (handle->rot_euler.x - (joystick.pointer.dy * dt * 50));
    handle->rot_euler.y = (handle->rot_euler.y - (joystick.pointer.dx * dt * 50));
    handle->rot_euler.z = 0;

    //clamp
    gl_t clamp = 80;
    if (clamp < handle->rot_euler.x)
        handle->rot_euler.x = clamp;
    else if (handle->rot_euler.x < -(clamp))
        handle->rot_euler.x = -(clamp);


    gl_mat roty = gl_mat_rotate_y(handle->rot_euler.y);
    gl_mat rotx = gl_mat_rotate_x(handle->rot_euler.x);
    gl_mat rot = gl_mat_mul(roty, rotx);
    gl_vec3 target = gl_mat_mul_vec(rot, fwd);
    gl_vec3 center = gl_vec3_add(handle->pos, target);
    handle->view = gl_mat_look_at(center, handle->pos, up);

    delta = gl_mat_mul_vec(rot, delta);
    delta = gl_vec3_mul(delta, gl_vec3_new_scalar(5 * dt));
    handle->pos = gl_vec3_add(delta, handle->pos);
}


void scene_draw(scene_handle handle) {

    //for now controller will be embedded, later positions of the nodes will be able to be changed
    controller_camera_fp_update(&controller_data, device_dt_get());
    for (int32_t i = 0; i < handle->meshes_count; ++i) {
        scene_mesh* mesh = handle->meshes + i;
        for (int32_t j = 0; j < mesh->primitives_count; ++j) {


            scene_mesh_primitive *primitive = mesh->primitives + j;
            gfx_pipeline_bind(primitive->pipeline_handle);
            scene_material *mat = handle->materials + primitive->material_id;
            if (primitive->material_id != -1 && mat->base.valid) {
                scene_texture *tex = handle->textures + mat->base.color_texture_id;
                if (mat->base.color_texture_id != -1 && tex->texture_handle != 0) {
                    gfx_texture_bind(tex->texture_handle, 0);
                }
                gfx_uniform_set(mat->color_factor_uniform, mat->base.color_factor);

                gfx_uniform_set(mat->model_uniform, mesh->world_tr.data);
                gfx_uniform_set(mat->view_uniform, controller_data.view.data);
                gfx_uniform_set(mat->projection_uniform, controller_data.projection.data);
            }

            if (handle->wireframe) {
                gfx_draw_id(GFX_LINES, primitive->indices_count);
            } else {
                gfx_draw_id(GFX_TRIANGLES, primitive->indices_count);
            }
        }
    }
    if(handle->bounding_box) dw_draw(handle->dw, controller_data.projection.data, controller_data.view.data);
}

scene_camera_projection scene_camera_projection_get() {
    scene_camera_projection result;
    os_memcpy(result.view, controller_data.view.data, sizeof(struct gl_mat));
    os_memcpy(result.projection, controller_data.projection.data, sizeof(struct gl_mat));
    return result;
}

void scene_delete(scene_handle handle) {
    for(int32_t i=0; i<handle->textures_count; ++i)
        gfx_texture_destroy(handle->textures[i].texture_handle);
    OS_FREE(handle->textures);
    OS_FREE(handle->materials);
    for(int32_t i=0; i<handle->meshes_count; ++i)
    {
        scene_mesh * mesh = handle->meshes + i;
        for(int32_t j=0; j<mesh->primitives_count; ++j)
        {
            scene_mesh_primitive* primitive = mesh->primitives + j;
            gfx_buffer_destroy(primitive->buffer_handle);
            gfx_buffer_destroy(primitive->index_handle);
            gfx_pipeline_destroy(primitive->pipeline_handle);
        }
        OS_FREE(mesh->primitives);
    }
    OS_FREE(handle->meshes);

    for(int32_t i=0; i<handle->nodes_count; ++i)
    {
        scene_node * node = handle->nodes + i;
        OS_FREE(node->children_id);
        OS_FREE(node->name);
    }
    gfx_shader_destroy(handle->shader);
    dw_delete(handle->dw);
    OS_FREE(handle->nodes);
    OS_FREE(handle);
}

void scene_wireframe_toggle(scene_handle handle) {
    handle->wireframe = !handle->wireframe;
}

void scene_bounding_box_toggle(scene_handle handle) {
    handle->bounding_box = !handle->bounding_box;
}

void scene_lighting_set(scene_handle handle, struct scene_lighting_settings lighting_settings) {
    handle->lighting_settings = lighting_settings;
}

void scene_sun_set(scene_handle handle, struct scene_sun_settings sun_settings) {
    handle->sun_settings = sun_settings;
}
