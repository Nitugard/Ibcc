/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_SCENE_H
#define IBCWEB_SCENE_H

#include <stdbool.h>
#include <stdint.h>

#ifndef IBC_API
#define IBC_API extern
#endif

typedef struct scene_skybox {
    bool render;
    const char* path;
} scene_skybox;

typedef struct scene_desc{
    scene_skybox skybox;
    void* model;
} scene_desc;

typedef struct scene_node {
    void *internal;
    char *name;
} scene_node;

typedef struct scene_camera{
    float projection[16];
    float fov;
    float znear;
    float zfar;
    float ar;
    bool perspective;
} scene_camera;

typedef struct scene_mesh{
    int32_t vertex_buffer_id;
} scene_mesh;


typedef struct scene_internal_data* scene_handle;

/*
 * Creation
 */
IBC_API scene_handle scene_new(scene_desc const* desc);
IBC_API void scene_delete(scene_handle handle);

/*
 * Editing.
 */
IBC_API void scene_draw(scene_handle handle);
IBC_API void scene_draw_with_camera(scene_handle handle, float projection[16], float tr[16], bool draw_skybox);

IBC_API void scene_node_count(scene_handle handle, int32_t* count);
IBC_API void scene_node_root_count(scene_handle handle, int32_t* count);
IBC_API void scene_node_children_count(scene_handle handle, scene_node* node, int32_t* count);
IBC_API void scene_camera_count(scene_handle handle, int32_t* count);
IBC_API void scene_mesh_count(scene_handle handle, int32_t* count);

IBC_API bool scene_node_get(scene_handle handle, const char* name, struct scene_node* node);
IBC_API void scene_node_get_at(scene_handle handle, int32_t index, struct scene_node *node);
IBC_API void scene_node_root_get_at(scene_handle handle, int32_t index, struct scene_node* node);
IBC_API void scene_node_children_get_at(scene_handle handle, int32_t index, struct scene_node* parent_node, scene_node* node);

IBC_API void scene_camera_get_at(scene_handle handle, int32_t index, struct scene_node* node, struct scene_camera* camera_data);
IBC_API void scene_camera_set(scene_handle handle, scene_node* node, scene_camera* camera_data);

IBC_API void scene_node_get_world_tr(scene_handle handle, scene_node* node, float* tr);
IBC_API void scene_node_set_world_tr(scene_handle handle, scene_node* node, float* tr);
IBC_API void scene_node_get_local_tr(scene_handle handle, scene_node* node, float* tr);
IBC_API void scene_node_set_local_tr(scene_handle handle, scene_node* node, float* tr);

#endif //IBCWEB_SCENE_H