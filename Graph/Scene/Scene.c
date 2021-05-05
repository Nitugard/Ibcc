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
#include <assert.h>


#define SCENE_DRAW_DEBUG_VERTEX_CAPACITY 16534

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

    gfx_uniform sun_color;
    gfx_uniform sun_direction;

    gfx_uniform diffuse_color;
    gfx_uniform ambient_color;

    gfx_uniform enable_shadows;
    gfx_uniform light_projection;
} scene_material;

typedef struct scene_mesh_primitive{
    int32_t indices_count;
    int32_t material_id;

    //todo: introduce one global buffer(pros: faster cons: removing nodes is not trivial/ impossible in case of static buffer?)
    gfx_pipeline_handle pipeline_handle;
    gfx_buffer_handle buffer_handle;
    gfx_buffer_handle index_handle;

    gfx_draw_type draw_type;
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
    gfx_texture_handle default_texture;


    scene_lighting_settings lighting_settings;
    scene_sun_settings sun_settings;

    bool enable_shadows;
    gfx_texture_handle shadow_depth_tex;
    gfx_pass_desc shadow_pass;
    gfx_framebuffer_handle fbo;
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

    //Todo: this should be one to map map(using switch case)
    result.draw_type = (uint32_t)primitive.primitive_type;

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
            attr->element_size = primitive.attributes[i].element_size;
            attr->elements_count = primitive.attributes[i].count;
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

scene_handle scene_new(scene_desc const* desc) {
    os_memset(&controller_data, 0, sizeof(controller_camera_data));

    int32_t width, height;
    device_window_dimensions_get(&width, &height);

    controller_data.pos = gl_vec3_new(0,1,-10);
    controller_data.projection = gl_mat_perspective(80, (float)width / height, 0.01, 100);

    mdl_data* model = desc->model;
    gfx_shader_handle sh_handle = gfx_shader_create(&lit_shader_desc);

    scene_handle handle = OS_MALLOC(sizeof(struct scene_data));
    os_memset(handle, 0, sizeof(scene_data));

    dw_desc dw_description = {};

    handle->shader = sh_handle;
    handle->dw = dw_new(&dw_description, SCENE_DRAW_DEBUG_VERTEX_CAPACITY);
    handle->sun_settings = desc->sun;
    handle->lighting_settings = desc->lighting;
    handle->enable_shadows = desc->enable_shadows;
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

        mat->model_uniform = gfx_uniform_register(handle->shader, STRING(MODEL), GFX_MAT4);
        mat->projection_uniform = gfx_uniform_register(handle->shader, STRING(PROJECTION), GFX_MAT4);
        mat->view_uniform = gfx_uniform_register(handle->shader, STRING(VIEW), GFX_MAT4);

        mat->diffuse_color = gfx_uniform_register(handle->shader, STRING(DIFFUSE_COLOR), GFX_FLOAT3);
        mat->ambient_color = gfx_uniform_register(handle->shader, STRING(AMBIENT_COLOR), GFX_FLOAT3);
        mat->sun_direction = gfx_uniform_register(handle->shader, STRING(SUN_DIRECTION), GFX_FLOAT3);
        mat->sun_color = gfx_uniform_register(handle->shader, STRING(SUN_COLOR), GFX_FLOAT3);
        mat->enable_shadows = gfx_uniform_register(handle->shader, STRING(ENABLE_SHADOWS), GFX_INT1);
        mat->light_projection = gfx_uniform_register(handle->shader, STRING(LIGHT_PROJECTION), GFX_MAT4);

        int tex0loc = 0;
        int tex1loc = 1;
        gfx_uniform tex0 = gfx_uniform_register(handle->shader, STRING(TEXTURE_MAIN), GFX_INT1);
        gfx_uniform tex1 = gfx_uniform_register(handle->shader, STRING(TEXTURE_SHADOW), GFX_INT1);

        gfx_uniform_set(tex0, &tex0loc);
        gfx_uniform_set(tex1, &tex1loc);

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

    /*
     * Create a shadow pass if necessary.
     */
    if(desc->enable_shadows)
    {
        struct gfx_texture_desc depth_tex = {
                .wrap = GFX_TEXTURE_WRAP_CLAMP,
                .mipmaps = false,
                .type = GFX_TEXTURE_TYPE_DEPTH,
                .width = desc->lighting.shadow_heightmap_size,
                .height = desc->lighting.shadow_heightmap_size,
                .data = 0,
                .filter = GFX_TEXTURE_FILTER_NEAREST
        };


        gfx_texture_handle depth_tex_handle = gfx_texture_create(&depth_tex);

        gfx_framebuffer_desc fbo_desc = {
                .depth_stencil_attachment = {.enabled = true, .texture_handle = depth_tex_handle}
        };

        gfx_framebuffer_handle fbo = gfx_framebuffer_create(&fbo_desc);

        gfx_pass_desc shadow_pass = {
                .fbo_handle = fbo,
                .actions = GFX_PASS_ACTION_CLEAR_DEPTH,
                .pass_options = GFX_PASS_OPTION_CULL_BACK | GFX_PASS_OPTION_DEPTH_TEST,
                .clear_color = {0, 0, 0, 1}
        };

        handle->shadow_depth_tex = depth_tex_handle;
        handle->shadow_pass = shadow_pass;
        handle->fbo = fbo;
    }
    gfx_texture_desc default_tex_desc = {
            .height = 256,
            .width = 256,
            .wrap = GFX_TEXTURE_WRAP_REPEAT,
            .mipmaps = false,
            .data = OS_MALLOC(256 * 256 * 4),
            .filter = GFX_TEXTURE_FILTER_NEAREST,
            .type = GFX_TEXTURE_TYPE_RGBA,
    };
    gfx_color white_color = {.x = 1, .y = 1, .z = 1, .w = 1};
    handle->default_texture = gfx_texture_create_color(&default_tex_desc, white_color);
    OS_FREE(default_tex_desc.data);
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

void scene_draw_pass(scene_handle handle, gl_mat projection, gl_mat view, gl_mat light_space, bool shadow_pass) {


    //Todo: shadow texture should always be passed to a shader at binding pointer zero, so other textures
    //Todo: have nice indexes, starting from one
    int shadows = handle->enable_shadows;
    for (int32_t i = 0; i < handle->meshes_count; ++i) {
        scene_mesh *mesh = handle->meshes + i;
        for (int32_t j = 0; j < mesh->primitives_count; ++j) {

            scene_mesh_primitive *primitive = mesh->primitives + j;
            gfx_pipeline_bind(primitive->pipeline_handle);
            scene_material *mat = handle->materials + primitive->material_id;
            if (primitive->material_id != -1 && mat->base.valid) {
                if(handle->textures_count != 0) {
                    scene_texture *tex = handle->textures + mat->base.color_texture_id;
                    if (mat->base.color_texture_id != -1 && tex->texture_handle != 0) {
                        gfx_texture_bind(tex->texture_handle, 0);
                    }
                }
                else{
                    gfx_texture_bind(handle->default_texture, 0);
                }

                gfx_texture_bind(handle->shadow_depth_tex, 1);
                gfx_uniform_set(mat->diffuse_color, mat->base.color_factor);
                gfx_uniform_set(mat->model_uniform, mesh->world_tr.data);
                gfx_uniform_set(mat->view_uniform, view.data);
                gfx_uniform_set(mat->projection_uniform, projection.data);
                gfx_uniform_set(mat->sun_direction, handle->sun_settings.direction);
                gfx_uniform_set(mat->sun_color, handle->sun_settings.color);
                gfx_uniform_set(mat->ambient_color, handle->lighting_settings.ambient_color);
                gfx_uniform_set(mat->enable_shadows, &shadows);
                gfx_uniform_set(mat->light_projection, light_space.data);

            } else {
                gfx_texture_bind(handle->default_texture, 0);
                gfx_texture_bind(handle->shadow_depth_tex, 1);
            }

            //todo: remove this
            if (handle->wireframe) {
                gfx_draw_id(GFX_LINES, primitive->indices_count);
            } else {
                gfx_draw_id(primitive->draw_type, primitive->indices_count);
            }
        }
    }
    if (handle->bounding_box) dw_draw(handle->dw, controller_data.projection.data, controller_data.view.data);
}


void scene_draw(scene_handle handle) {
    int32_t w, h;
    //for now controller will be embedded, later positions of the nodes will be able to be changed
    device_window_dimensions_get(&w, &h);
    controller_camera_fp_update(&controller_data, device_dt_get());
    gl_mat light_space =GL_MAT_IDENTITY;
    if (handle->enable_shadows) {
        //calculate projection and view in order to create camera projection matrix
        gl_mat view, projection;
        gl_vec3 light_dir = gl_vec3_new_arr(handle->sun_settings.direction);
        light_dir.x = gl_sin(device_time_get() / 10)/2;
        light_dir.z = -gl_cos(device_time_get() / 10);
        light_dir.y = -1;
        view = gl_mat_look_at(gl_vec3_new(0,0,0), gl_vec3_mul((gl_vec3_normalize(light_dir)), gl_vec3_new_scalar(-10)), gl_vec3_new(0,1,0));
        projection = gl_mat_ortographic(-10, 10, -10, 10, 0.1, 100);
        light_space = GL_MAT_MUL_LR(projection, view);
        gfx_viewport_set(handle->lighting_settings.shadow_heightmap_size, handle->lighting_settings.shadow_heightmap_size);
        gfx_begin_pass(&handle->shadow_pass);
        scene_draw_pass(handle, projection, view, light_space, true);
        gfx_end_pass();
    }
    gfx_viewport_set(w, h);
    scene_draw_pass(handle, controller_data.projection, controller_data.view, light_space, false);
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
    if(handle->enable_shadows){
        gfx_texture_destroy(handle->shadow_depth_tex);
        gfx_framebuffer_destroy(handle->fbo);
    }
    gfx_texture_destroy(handle->default_texture);
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
    assert(handle->lighting_settings.shadow_heightmap_size == lighting_settings.shadow_heightmap_size);
    handle->lighting_settings = lighting_settings;
}

void scene_sun_set(scene_handle handle, struct scene_sun_settings sun_settings) {
    handle->sun_settings = sun_settings;
}

void *scene_directional_lighting_depth_texture_get(scene_handle handle) {
    assert(handle->enable_shadows);
    return handle->shadow_depth_tex;
}

struct scene_inode scene_node_get(scene_handle handle, const char *name) {
    scene_inode result;
    for(int32_t i=0; i<handle->nodes_count; ++i) {
        if (handle->nodes[i].name == 0) continue;
        if (strcmp(handle->nodes[i].name, name) == 0) {
            result.name = handle->nodes[i].name;
            result.children_count = handle->nodes[i].children_count;
            result.internal = handle->nodes + i;
            os_memcpy(result.local, handle->nodes[i].local_tr.data, sizeof(gl_t) * 16);
            return result;
        }
    }
    result.name = 0;
    result.internal = 0;
    return result;
}

void scene_node_update(scene_handle handle, scene_inode *node) {
    if(node->internal == 0) {
        //todo: log error
        return;
    }
    scene_node *node_int = (scene_node *) node->internal;
    os_memcpy(node_int->local_tr.data, node->local, sizeof(gl_mat));

    //update mesh local transform
    if(node_int->mesh_index != -1)
        os_memcpy(handle->meshes[node_int->mesh_index].world_tr.data, node->local, sizeof(gl_mat));
}


