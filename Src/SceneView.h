/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_SCENEVIEW_H
#define IBCWEB_SCENEVIEW_H

#include "stdbool.h"
#include "inttypes.h"

#ifndef IBC_API
#define IBC_API extern
#endif

typedef struct scene_view* scene_view_handle;

typedef enum scene_view_type{
    SCENE_VIEW_PERSPECTIVE,
    SCENE_VIEW_ORTOGRAPHIC_FRONT,
} scene_view_type;

typedef struct scene_view_camera_options{
    float fov;
    float znear;
    float zfar;
} scene_view_camera_options;

typedef struct scene_view_controller_options{
    float move_speed;
    float rotate_speed;
} scene_view_controller_options;

IBC_API scene_view_handle scene_view_create(int32_t width, int32_t height, scene_view_type view_type);

IBC_API void scene_view_set_tr(scene_view_handle handle, float tr[16]);

IBC_API void scene_view_render(scene_view_handle handle, void* scene);
IBC_API void scene_view_update_controller(scene_view_handle handle);
IBC_API void scene_view_resize(scene_view_handle handle, int32_t w, int32_t h);
IBC_API void scene_view_render_get(scene_view_handle handle, int32_t* color, int32_t* depth);
IBC_API void scene_view_destroy(scene_view_handle handle);
IBC_API void scene_view_flag_dirty(scene_view_handle handle);
IBC_API void scene_view_get_size(scene_view_handle handle, int32_t* width, int32_t* height);
IBC_API char const* scene_view_get_name(scene_view_handle handle);

#endif //IBCWEB_SCENEVIEW_H
