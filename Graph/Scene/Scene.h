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

//Todo: light intensity!!
#define SCENE_DEFAULT_LIGHTING {.ambient_color = {0.7, 0.7, 0.7}, .ambient_intensity = 0, .shadow_heightmap_size = 512}
#define SCENE_DEFAULT_SUN {.direction = {-0.5, -1, 0.5}, .color = {1, 1, 0.9}, .intensity = 0.7}

typedef struct scene_lighting_settings{
    float ambient_color[3];
    float ambient_intensity; //Todo
    bool shadows;
    int32_t shadow_heightmap_size;
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

typedef struct scene_inode{
    void const* internal;
    char* name;
    float local[16];
    int32_t children_count;
} scene_inode;

typedef struct scene_camera_data scene_camera_handle;
typedef struct scene_data* scene_handle;

/*
 * Creation
 */
API scene_handle scene_new(scene_desc const* desc);
API void scene_delete(scene_handle handle);


/*
 * Editing.
 */
API void scene_draw(scene_handle handle);
API void scene_wireframe_toggle(scene_handle handle);
API void scene_bounding_box_toggle(scene_handle handle);
API scene_inode scene_node_get(scene_handle handle, const char* name);
API void scene_node_update(scene_handle handle, scene_inode* node);
API scene_inode scene_main_camera_get(scene_handle handle);

/*
 * Lighting.
 */
API void scene_lighting_set(scene_handle handle, struct scene_lighting_settings lighting_settings);
API void scene_sun_set(scene_handle handle, struct scene_sun_settings sun_settings);
API void* scene_directional_lighting_depth_texture_get(scene_handle handle);


/*
 * Controller.
 */
API scene_camera_projection scene_camera_projection_get();

#endif //IBC_SCENE_H
