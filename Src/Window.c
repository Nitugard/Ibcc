/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <assert.h>
#include <string.h>
#include <time.h>

#include "Window.h"
#include "Device.h"
#include "Gui.h"
#include "Graphics.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "Allocator.h"
#include "Scene.h"
#include "Model.h"
#include "SceneView.h"
#include "GlMath.h"

#define MAXIMUM_WINDOW_LOGS 1024

typedef struct window_shader_document{
    gfx_shader_handle handle;
    char* name;
    char* buffer_vs;
    char* buffer_fs;
    bool open;
    bool dirty;
    bool want_close;
    bool open_prev;
    size_t buffer_length;
} window_shader_document;

typedef struct window_log_data{
    char* log;
    time_t time;
    int32_t length;
} window_log_data;

window_log_data logs[MAXIMUM_WINDOW_LOGS];
int32_t logs_count;

window_shader_document shaders[MAXIMUM_LOADED_SHADERS];
scene_view_handle views[2];

scene_handle active_scene;
mdl_handle model;
device_handle device;

float bg_color[4] = {0.12, 0.11, 0.12, 1.0f};

bool window_shader_find(gfx_shader_handle handle, int32_t* index) {
    *index = -1;
    for (int32_t i = 0; i < MAXIMUM_LOADED_SHADERS; ++i) {
        if(shaders[i].handle == 0 && *index == -1)
            *index = i;
        if (shaders[i].handle == handle) {
            *index = i;
            return true;
        }
    }
    return false;
}

bool window_shader_find_by_name(gfx_shader_handle handle, int32_t* index) {
    for (int32_t i = 0; i < MAXIMUM_LOADED_SHADERS; ++i) {
        char* n1, *n2;
        gfx_shader_name_get(shaders[i].handle, &n1);
        gfx_shader_name_get(handle, &n2);

        if (strcmp(n1, n2) == 0) {
            *index = i;
            return true;
        }
    }
    return false;
}

void window_on_gfx_log(char const* log, bool error) {
    if (logs_count == MAXIMUM_WINDOW_LOGS) {
        OS_FREE(logs[0].log);
        for (int32_t i = 1; i < MAXIMUM_WINDOW_LOGS; ++i)
            logs[i - 1] = logs[i];
        logs_count -= 1;
    }
    logs[logs_count].length = strlen(log);
    logs[logs_count].log = OS_MALLOC(logs[logs_count].length + 1);
    logs[logs_count].time = time(NULL);
    os_memcpy(logs[logs_count].log, log, logs[logs_count].length + 1);
    logs_count++;
}

void window_on_shader_change(gfx_shader_handle handle){

    switch (gfx_shader_status(handle)) {
        case GFX_RESOURCE_INVALID:
            break;
        case GFX_RESOURCE_DESTROYED: {
            int32_t index;
            if(window_shader_find(handle, &index)) {
                shaders[index].handle = 0;
                shaders[index].want_close = true;
                OS_FREE(shaders[index].buffer_vs);
                OS_FREE(shaders[index].buffer_fs);
            }
        }
            break;
        case GFX_RESOURCE_CREATED:
            break;
        case GFX_RESOURCE_ACTIVE: {
            int32_t index;
            if (!window_shader_find(handle, &index)) {

                shaders[index].handle = handle;
                shaders[index].want_close = false;
                shaders[index].open = true;
                shaders[index].open_prev = false;
                shaders[index].dirty = false;

                char *internal_buffer_vs, *internal_buffer_fs;
                gfx_shader_vs_get(handle, &internal_buffer_vs);
                gfx_shader_fs_get(handle, &internal_buffer_fs);
                gfx_shader_name_get(handle, &shaders[index].name);

                shaders[index].buffer_vs = OS_MALLOC(1024 * 1024);
                shaders[index].buffer_fs = OS_MALLOC(1024 * 1024);
                shaders[index].buffer_length = 1024 * 1024;

                os_memcpy(shaders[index].buffer_vs, internal_buffer_vs, strlen(internal_buffer_vs) + 1);
                os_memcpy(shaders[index].buffer_fs, internal_buffer_fs, strlen(internal_buffer_fs) + 1);
            }
        }
            break;
    }
}

int window_shader_editor_callback(struct ImGuiInputTextCallbackData* data) {
    return 0;
}

void window_scene_views_mark_as_dirty(){
    for(int32_t j=0; j<sizeof(views) / sizeof(void*); ++j)
    {
        scene_view_flag_dirty(views[j]);
    }
}

void window_shader_editor(){
    igBegin("Shader Editor", 0, 0);
    char name_buffer[1024];
    if(igBeginTabBar("##tabs", ImGuiTabBarFlags_Reorderable)) {
        for (int32_t i = 0; i < MAXIMUM_LOADED_SHADERS; ++i) {
            struct window_shader_document *doc = shaders + i;
            if (doc->handle != 0 && doc->open) {
                sprintf(name_buffer, "[%i] %s", i, doc->name);
                ImGuiTabItemFlags_ flags = doc->dirty ? (ImGuiTabItemFlags_UnsavedDocument | ImGuiTabItemFlags_NoCloseButton): ImGuiTabItemFlags_NoCloseButton;
                bool visible = igBeginTabItem(name_buffer, &doc->open, flags);

                if(!doc->open && doc->dirty) {
                    doc->open = true;
                    doc->want_close = true;
                }


                if (visible) {
                    if(igInputTextMultiline("##FragShader", doc->buffer_fs, doc->buffer_length, (struct ImVec2) {igGetWindowWidth()/2, igGetWindowHeight() }, ImGuiInputTextFlags_Multiline |ImGuiInputTextFlags_AutoSelectAll,
                                         0, &shaders[i])) {
                        doc->dirty = true;
                    }
                    igSameLine(igGetWindowWidth()/2, 5);
                    if(igInputTextMultiline("##VertShader", doc->buffer_vs, doc->buffer_length, (struct ImVec2) {igGetWindowWidth()/2, igGetWindowHeight() }, ImGuiInputTextFlags_Multiline |ImGuiInputTextFlags_AutoSelectAll,
                                            0, &shaders[i])) {
                        doc->dirty = true;
                    }

                    if(doc->dirty) {
                        gfx_shader_reload(doc->handle);
                        gfx_shader_add_vs(doc->handle, doc->buffer_vs);
                        gfx_shader_add_fs(doc->handle, doc->buffer_fs);
                        gfx_shader_submit(doc->handle);
                        doc->dirty = false;
                        window_scene_views_mark_as_dirty();
                    }

                    igEndTabItem();
                }
            }
        }

        igEndTabBar();
    }

    igEnd();
}

void window_scene_view_create(){

    views[0] = scene_view_create(32, 32, SCENE_VIEW_PERSPECTIVE);
    views[1] = scene_view_create(32, 32, SCENE_VIEW_ORTOGRAPHIC_FRONT);

}

void window_scene_view_draw(){
    int32_t color;
    for(int32_t i=0; i<sizeof(views) / sizeof(void*); ++i) {

        ImGuiWindowFlags_ flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
                                  | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysUseWindowPadding
                                  | ImGuiWindowFlags_NoBringToFrontOnFocus;
        igBegin(scene_view_get_name(views[i]), 0, flags);


        if (igIsWindowHovered(0)) {
            scene_view_update_controller(views[i]);
        }

        scene_view_render(views[i], active_scene);
        scene_view_render_get(views[i], &color, 0);

        int32_t x, y;
        scene_view_get_size(views[i], &x, &y);

        igImage((void *) (intptr_t) color, (ImVec2) {x, y}, (struct ImVec2) {0, 1}, (struct ImVec2) {1, 0},
                (struct ImVec4) {1, 1, 1, 1}, (struct ImVec4) {0, 0, 0, 0});

        ImVec2 size;
        igGetWindowContentRegionMax(&size);
        scene_view_resize(views[i], size.x, size.y);
        igEnd();
    }

}

void window_log(){
    igBegin("Log", 0, 0);
    char timeBuffer[1024];
    for(int i=logs_count-1; i>=0; --i) {
        struct tm* timeinfo = localtime(&logs[i].time);
        sprintf(timeBuffer, "[%dh:%dm:%ds]", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        igTextUnformatted(timeBuffer, timeBuffer + strlen(timeBuffer));
        igSameLine(100, 0);
        igTextUnformatted(logs[i].log, logs[i].log + logs[i].length);
    }
    igEnd();
}

gl_t nau02_loc = 1.38;
int32_t nauo2_side = 1;

float nauo2_speed = 1.0f;
float nau01_rot_speed = 100.0f;

float nau01_rot = 0.0f;
float nau01_rot_dir = 1.0f;
float rot_limit = 180.0f;

const float nauo_speed = 1.0f;
const float start_height = 2.857;
const float end_height = 2.408;
float target_height = start_height;
int32_t nauo_side = 1;

bool hold = false;
bool release = false;
const float tight = -0.031;
const float relax_delta = 0.055;
const float speed = 0.05f;
void window_manipulator_demo(){

    const float epsilon = .0001f;
    scene_node nauo2;
    if(scene_node_get(active_scene, "NAUO2", &nauo2));
    gl_mat tr;
    scene_node_get_world_tr(active_scene, &nauo2, tr.data);
    gl_vec3 cur_tr = gl_mat_get_translation(tr);
    gl_t delta = -cur_tr.x + nauo2_side * nau02_loc;
    if(gl_abs(delta) > epsilon){
        gl_t ndelta = delta / gl_abs(delta);
        float new_pos = cur_tr.x + nauo2_speed * device_dt_get() * ndelta;
        new_pos = gl_clamp(new_pos, -nau02_loc, nau02_loc);
        if(gl_abs(new_pos - cur_tr.x) > epsilon) {
            tr = gl_mat_set_translation(tr, gl_vec3_new(new_pos, cur_tr.y, cur_tr.z));
            scene_node_set_world_tr(active_scene, &nauo2, tr.data);
            window_scene_views_mark_as_dirty();
        }
    }

    scene_node nauo;
    if(scene_node_get(active_scene, "NAUO2.001", &nauo));
    scene_node_get_world_tr(active_scene, &nauo, tr.data);
    cur_tr = gl_mat_get_translation(tr);
    delta = (- cur_tr.y + target_height);
    if(gl_abs(delta) > epsilon) {
        gl_t ndelta = delta / gl_abs(delta);
        gl_t new_height = cur_tr.y + ndelta * nauo_speed * device_dt_get();
        new_height = gl_clamp(new_height, end_height, start_height);
        if (gl_abs(cur_tr.y - new_height) > epsilon) {
            tr = gl_mat_set_translation(tr, gl_vec3_new(cur_tr.x, new_height, cur_tr.z));
            scene_node_set_world_tr(active_scene, &nauo, tr.data);
            window_scene_views_mark_as_dirty();
        }
    }

    scene_node nauo1;
    if(scene_node_get(active_scene, "NAUO1.003", &nauo1));
    scene_node_get_world_tr(active_scene, &nauo1, tr.data);
    if(gl_abs(nau01_rot_dir) > epsilon){

        nau01_rot += device_dt_get() * nau01_rot_speed * nau01_rot_dir;
        gl_vec3 cur_tr = gl_mat_get_translation(tr);
        gl_mat new_tr = gl_mat_rotate_y(nau01_rot);
        new_tr = gl_mat_set_translation(new_tr, cur_tr);
        nau01_rot = gl_clamp(nau01_rot, -rot_limit, rot_limit);

        scene_node_set_world_tr(active_scene, &nauo1, new_tr.data);
        window_scene_views_mark_as_dirty();
    }


    scene_node n1, n2;
    gl_mat tr1, tr2;
    gl_vec3 cur_tr1, cur_tr2;
    if(scene_node_get(active_scene, "NAUO3", &n1));
    if(scene_node_get(active_scene, "NAUO2.002", &n2));
    scene_node_get_local_tr(active_scene, &n1, tr1.data);
    scene_node_get_local_tr(active_scene, &n2, tr2.data);
    cur_tr1 = gl_mat_get_translation(tr1);
    cur_tr2 = gl_mat_get_translation(tr2);

    if(hold)
    {
        cur_tr1 = gl_vec3_add(cur_tr1, gl_vec3_new(0,0, speed * device_dt_get()));
        cur_tr2 = gl_vec3_add(cur_tr2, gl_vec3_new(0,0, -speed * device_dt_get()));
    }

    if(release){
        cur_tr1 = gl_vec3_add(cur_tr1, gl_vec3_new(0,0, -speed * device_dt_get()));
        cur_tr2 = gl_vec3_add(cur_tr2, gl_vec3_new(0,0, speed * device_dt_get()));
    }

    if(hold || release) {
        cur_tr1.z = gl_clamp(cur_tr1.z, tight - relax_delta, tight);
        cur_tr2.z = gl_clamp(cur_tr2.z, tight, tight + relax_delta);
        tr1 = gl_mat_set_translation(tr1, cur_tr1);
        tr2 = gl_mat_set_translation(tr2, cur_tr2);
        scene_node_set_local_tr(active_scene, &n1, tr1.data);
        scene_node_set_local_tr(active_scene, &n2, tr2.data);
        window_scene_views_mark_as_dirty();
    }

    if(igBegin("Manipulator",0,ImGuiWindowFlags_NoBackground)) {
        if (igButton("Translate X", (struct ImVec2) {200, 25})) {
            nauo2_side *= -1;
        }

        bool active = false;
        igButton("CW", (struct ImVec2) {80, 25});
        if (igIsItemActive()) {
            nau01_rot_dir = -1.0f;
            active = true;
        }
        igSameLine(127, 0);
        igButton("CCW", (struct ImVec2) {80, 25});
        if (igIsItemActive()) {
            nau01_rot_dir = 1.0f;
            active = true;
        }

        if (!active) nau01_rot_dir = 0.0f;

        if (igButton("Translate Y", (struct ImVec2) {200, 25})) {
            nauo_side *= -1.0;
            if (nauo_side > 0)
                target_height = start_height;
            else target_height = end_height;

        }

        igButton("Hold", (struct ImVec2) {200, 25});
        hold = (igIsItemActive());

        (igButton("Release", (struct ImVec2) {200, 25}));
        release = igIsItemActive();
        igEnd();
    }
}

void window_init(struct window_config const* config) {
    device_init(3, 0);
    device = device_new(config->title, config->width, config->height, config->vsync, config->fullscreen,
                        config->resizable);
    device_set_current(device);
    gfx_init();
    gui_init();

    os_memset(shaders, 0, sizeof(struct window_shader_document) * MAXIMUM_LOADED_SHADERS);

    gfx_log_callback_set(window_on_gfx_log);
    gfx_shader_status_change_callback_set(window_on_shader_change);

    model = mdl_load("./Data/Manipulator.gltf");
    scene_desc desc = {
            .skybox = {
                    .path = "./Data/skybox/",
                    .render = true,
            },
            .model = model,
    };

    active_scene = scene_new(&desc);
    mdl_unload(model);

    window_scene_view_create();
    os_memset(logs, 0, sizeof(logs));
    logs_count=0;
}

int32_t color;
void window_run() {
    while(device_window_valid()) {

        device_update_events();


        gfx_begin_pass(0, GFX_PASS_OPTION_DEPTH_TEST, GFX_PASS_ACTION_CLEAR_COLOR | GFX_PASS_ACTION_CLEAR_DEPTH,
                       bg_color);
        gui_begin_frame();
        window_scene_view_draw();
        window_shader_editor();
        window_manipulator_demo();
        window_log();
        igShowMetricsWindow(0);
        gui_end_frame();
        gfx_end_pass();
        device_refresh();
    }
}

void window_finalize(){
    scene_delete(active_scene);
    gui_finalize();
    gfx_terminate();
    device_delete(device);
    device_terminate();
}
