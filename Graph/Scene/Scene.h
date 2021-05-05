/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_SCENE_H
#define IBC_SCENE_H

#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

#define SCENE_DEFAULT_LIGHTING {.ambient_color = {1, 1, 1}, .ambient_intensity = 0.7}
#define SCENE_DEFAULT_SUN {.direction = {-0.5, -1, 0.5}, .color = {1, 1, 0.9}, .intensity = 0.7}

typedef struct scene_lighting_settings{
    float ambient_color[3];
    float ambient_intensity; //Todo
    bool shadows;
} scene_lighting_settings;

typedef struct scene_sun_settings{
    float direction[3];
    float color[3];
    float intensity; //Todo
} scene_sun_settings;

typedef struct scene_camera_projection{
    float projection[16];
    float view[16];
} scene_camera_projection;

typedef struct scene_desc{
    bool enable_shadows;
    struct scene_sun_settings sun;
    struct scene_lighting_settings lighting;
    void* model;
} scene_desc;

typedef struct scene_camera_data scene_camera_handle;
typedef struct scene_data* scene_handle;

API scene_handle scene_new(scene_desc const* desc);
API void scene_draw(scene_handle handle);
API void scene_delete(scene_handle handle);
API void scene_wireframe_toggle(scene_handle handle);
API void scene_bounding_box_toggle(scene_handle handle);
API void* scene_get_lighting_depth_texture(scene_handle);
API void scene_lighting_set(scene_handle handle, struct scene_lighting_settings lighting_settings);
API void scene_sun_set(scene_handle handle, struct scene_sun_settings sun_settings);

API scene_camera_projection scene_camera_projection_get( );

#endif //IBC_SCENE_H
