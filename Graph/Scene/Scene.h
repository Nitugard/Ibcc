/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_SCENE_H
#define IBC_SCENE_H

#ifndef API
#define API
#endif

#define SCENE_DEFAULT_LIGHTING {.ambient_color = {0.25, 0.25, 0.3}, .ambient_intensity = 0.7};
#define SCENE_DEFAULT_SUN {.direction = {-0.7, -1, 0.7}, .color = {1, 1, 0.9}, .intensity = 0.7};

typedef struct scene_lighting_settings{
    float ambient_color[3];
    float ambient_intensity;
} scene_lighting_settings;

typedef struct scene_sun_settings{
    float direction[3];
    float color[3];
    float intensity;
} scene_sun_settings;

typedef struct scene_camera_projection{
    float projection[16];
    float view[16];
} scene_camera_projection;

typedef struct scene_camera_data scene_camera_handle;
typedef struct scene_data* scene_handle;

API scene_handle scene_new(void* model, struct scene_sun_settings sun_settings, struct scene_lighting_settings lighting_settings);
API void scene_draw(scene_handle handle);
API void scene_delete(scene_handle handle);
API void scene_wireframe_toggle(scene_handle handle);
API void scene_bounding_box_toggle(scene_handle handle);
API void scene_lighting_set(scene_handle handle, struct scene_lighting_settings lighting_settings);
API void scene_sun_set(scene_handle handle, struct scene_sun_settings sun_settings);

API scene_camera_projection scene_camera_projection_get( );

#endif //IBC_SCENE_H
