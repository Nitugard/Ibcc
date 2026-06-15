/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <assert.h>
#include <stdio.h>
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

typedef struct {
    const char* project_name;
    const char* student_name;
    const char* professor_name;
    const char* course_name;
    const char* faculty_name;
} window_project_info;

static window_project_info project_info = {
    .project_name  = "Graficki prikaz manipulatora",
    .student_name  = "Dragutin Sredojevic",
    .professor_name= "Dusan Nedeljkovic",
    .course_name   = "Kompjuterska Grafika",
    .faculty_name  = "Masinski Fakultet",
};

static float frame_dt      = 0.0f;
static bool  show_dev_tools = false;

float bg_color[4] = {0.09f, 0.09f, 0.12f, 1.0f};

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
    if (views[0]) scene_view_flag_dirty(views[0]);
}

void window_shader_editor(){
    int32_t width, height;
    device_window_dimensions_get(&width, &height);
    igSetNextWindowPos((ImVec2){8, height * 0.62f}, ImGuiCond_FirstUseEver, (ImVec2){0, 0});
    igSetNextWindowSize((ImVec2){width * 0.58f, height * 0.36f}, ImGuiCond_FirstUseEver);
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

                    if(doc->dirty && igButton("Apply", (ImVec2){100, 25})) {
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
    views[1] = NULL;
}

static void window_view_options(void)
{
    float fov = scene_view_get_fov(views[0]);
    if (igSliderFloat("FOV", &fov, 10.0f, 120.0f, "%.1f", 0))
        scene_view_set_fov(views[0], fov);
}

static void window_manipulator_controls(void);   /* defined after motion vars */

void window_scene_view_draw(){
    int32_t ww, wh;
    device_window_dimensions_get(&ww, &wh);

    /* Right panel: wide enough for text + buttons (min 260 px). */
    float panel_w = gl_max((float)ww * 0.23f, 260.0f);
    float view_w  = (float)ww - panel_w;

    /* ------------------------------------------------------------------
     * Left: 3D perspective view — borderless, no title bar, fills left side.
     * ------------------------------------------------------------------ */
    igSetNextWindowPos((ImVec2){0.0f, 0.0f}, ImGuiCond_Always, (ImVec2){0.0f, 0.0f});
    igSetNextWindowSize((ImVec2){view_w, (float)wh}, ImGuiCond_Always);
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2){0.0f, 0.0f});
    {
        ImGuiWindowFlags_ vf =
            ImGuiWindowFlags_NoDecoration      |
            ImGuiWindowFlags_NoMove            |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoSavedSettings   |
            ImGuiWindowFlags_NoBackground;
        igBegin("##3dview", NULL, vf);

        if (igIsWindowHovered(0)) {
            scene_view_update_controller(views[0]);
        }

        ImVec2 avail;
        igGetContentRegionAvail(&avail);
        if (avail.x >= 1.0f && avail.y >= 1.0f) {
            scene_view_resize(views[0], (int32_t)avail.x, (int32_t)avail.y);
            scene_view_render(views[0], active_scene);

            int32_t color_tex = -1;
            scene_view_render_get(views[0], &color_tex, 0);

            int32_t sx, sy;
            scene_view_get_size(views[0], &sx, &sy);
            igImage(
                (void*)(intptr_t)color_tex,
                (ImVec2){(float)sx, (float)sy},
                (ImVec2){0, 1}, (ImVec2){1, 0},
                (ImVec4){1, 1, 1, 1}, (ImVec4){0, 0, 0, 0}
            );
        }
        igEnd();
    }
    igPopStyleVar(1);

    /* ------------------------------------------------------------------
     * Right: presentation panel — project info, camera, controls, stats.
     * ------------------------------------------------------------------ */
    igSetNextWindowPos((ImVec2){view_w, 0.0f}, ImGuiCond_Always, (ImVec2){0.0f, 0.0f});
    igSetNextWindowSize((ImVec2){panel_w, (float)wh}, ImGuiCond_Always);
    {
        ImGuiWindowFlags_ pf =
            ImGuiWindowFlags_NoTitleBar    |
            ImGuiWindowFlags_NoResize      |
            ImGuiWindowFlags_NoMove        |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoSavedSettings;
        igBegin("##panel", NULL, pf);

        /* ---- Project identity ---- */
        igSpacing();
        igTextColored((ImVec4){0.50f, 0.78f, 1.00f, 1.00f}, "%s", project_info.course_name);
        igTextColored((ImVec4){0.70f, 0.70f, 0.76f, 1.00f}, "%s", project_info.faculty_name);
        igSpacing();
        igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){1.0f, 1.0f, 1.0f, 1.0f});
        igTextWrapped("%s", project_info.project_name);
        igPopStyleColor(1);
        igSpacing();
        igText("Student:  %s", project_info.student_name);
        igText("Profesor: %s", project_info.professor_name);
        igSeparator();

        /* ---- Camera ---- */
        igTextDisabled("KAMERA");
        window_view_options();
        igSeparator();

        /* ---- Manipulator controls ---- */
        igTextDisabled("MANIPULATOR");
        igSpacing();
        window_manipulator_controls();
        igSeparator();

        /* ---- Camera control hints ---- */
        igTextDisabled("KONTROLE");
        igText("Desni klik    Rotacija");
        igText("Srednji klik  Pomak");
        igText("Tocak         Zum");
        igSeparator();

        /* ---- Stats ---- */
        igTextDisabled("STATISTIKE");
        {
            int32_t nodes = 0, meshes = 0;
            scene_node_count(active_scene, &nodes);
            scene_mesh_count(active_scene, &meshes);
            igText("Cvorovi: %d   Mreze: %d", nodes, meshes);
            igText("FPS: %.0f", frame_dt > 0.00001f ? 1.0f / frame_dt : 0.0f);
        }
        igSeparator();

        /* ---- Developer tools toggle ---- */
        if (igButton(show_dev_tools ? "Sakrij alate" : "Dev alati", (ImVec2){-1.0f, 0.0f}))
            show_dev_tools = !show_dev_tools;

        igEnd();
    }
}

void window_log(){
    int32_t width, height;
    device_window_dimensions_get(&width, &height);
    igSetNextWindowPos((ImVec2){width * 0.59f, height * 0.62f}, ImGuiCond_FirstUseEver, (ImVec2){0, 0});
    igSetNextWindowSize((ImVec2){width * 0.20f, height * 0.36f}, ImGuiCond_FirstUseEver);
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

static void window_manipulator_controls(void)
{
    ImVec2 avail; igGetContentRegionAvail(&avail);
    float hw = (avail.x - igGetStyle()->ItemSpacing.x) * 0.5f;

    if (igButton("Pomak X", (ImVec2){-1.0f, 0.0f}))
        nauo2_side *= -1;

    bool rot_active = false;
    igButton("CW",  (ImVec2){hw, 0.0f}); if (igIsItemActive()) { nau01_rot_dir = -1.0f; rot_active = true; }
    igSameLine(0.0f, -1.0f);
    igButton("CCW", (ImVec2){hw, 0.0f}); if (igIsItemActive()) { nau01_rot_dir =  1.0f; rot_active = true; }
    if (!rot_active) nau01_rot_dir = 0.0f;

    if (igButton("Pomak Y", (ImVec2){-1.0f, 0.0f})) {
        nauo_side *= -1;
        target_height = (nauo_side > 0) ? start_height : end_height;
    }

    igButton("Stisni",  (ImVec2){-1.0f, 0.0f}); hold    = igIsItemActive();
    igButton("Otpusti", (ImVec2){-1.0f, 0.0f}); release = igIsItemActive();
}

static bool window_get_required_node(const char* name, scene_node* out_node)
{
    /*
     * Centralized node lookup for the manipulator demo.
     * If a node name changes in Blender/glTF export, this function prevents
     * silent memory corruption and gives a clear console message.
     */
    if (active_scene == 0 || out_node == 0 || name == 0) {
        fprintf(stderr, "Manipulator demo: invalid node lookup request\n");
        return false;
    }

    if (!scene_node_get(active_scene, name, out_node)) {
        fprintf(stderr, "Manipulator demo: required node '%s' was not found\n", name);
        return false;
    }

    return true;
}

void window_manipulator_demo()
{
    const float epsilon = 0.0001f;
    const float dt = device_dt_get();
    frame_dt = dt;

    /*
     * Translate carriage on X.
     */
    scene_node nauo2;
    if (!window_get_required_node("NAUO2", &nauo2)) {
        return;
    }

    gl_mat tr;
    scene_node_get_world_tr(active_scene, &nauo2, tr.data);

    gl_vec3 cur_tr = gl_mat_get_translation(tr);
    gl_t delta = -cur_tr.x + nauo2_side * nau02_loc;

    if (gl_abs(delta) > epsilon) {
        gl_t direction = delta / gl_abs(delta);
        float new_pos = cur_tr.x + nauo2_speed * dt * direction;
        new_pos = gl_clamp(new_pos, -nau02_loc, nau02_loc);

        if (gl_abs(new_pos - cur_tr.x) > epsilon) {
            tr = gl_mat_set_translation(tr, gl_vec3_new(new_pos, cur_tr.y, cur_tr.z));
            scene_node_set_world_tr(active_scene, &nauo2, tr.data);
            window_scene_views_mark_as_dirty();
        }
    }

    /*
     * Translate vertical/lift node on Y.
     */
    scene_node nauo;
    if (!window_get_required_node("NAUO2.001", &nauo)) {
        return;
    }

    scene_node_get_world_tr(active_scene, &nauo, tr.data);
    cur_tr = gl_mat_get_translation(tr);
    delta = -cur_tr.y + target_height;

    if (gl_abs(delta) > epsilon) {
        gl_t direction = delta / gl_abs(delta);
        gl_t new_height = cur_tr.y + direction * nauo_speed * dt;
        new_height = gl_clamp(new_height, end_height, start_height);

        if (gl_abs(cur_tr.y - new_height) > epsilon) {
            tr = gl_mat_set_translation(tr, gl_vec3_new(cur_tr.x, new_height, cur_tr.z));
            scene_node_set_world_tr(active_scene, &nauo, tr.data);
            window_scene_views_mark_as_dirty();
        }
    }

    /*
     * Rotate wrist/gripper.
     * Important fix: clamp rotation before building the matrix.
     */
    scene_node nauo1;
    if (!window_get_required_node("NAUO1.003", &nauo1)) {
        return;
    }

    scene_node_get_world_tr(active_scene, &nauo1, tr.data);

    if (gl_abs(nau01_rot_dir) > epsilon) {
        nau01_rot += dt * nau01_rot_speed * nau01_rot_dir;
        nau01_rot = gl_clamp(nau01_rot, -rot_limit, rot_limit);

        gl_vec3 current_position = gl_mat_get_translation(tr);
        gl_mat new_tr = gl_mat_rotate_y(nau01_rot);
        new_tr = gl_mat_set_translation(new_tr, current_position);

        scene_node_set_world_tr(active_scene, &nauo1, new_tr.data);
        window_scene_views_mark_as_dirty();
    }

    /*
     * Move gripper fingers.
     */
    scene_node n1, n2;
    if (!window_get_required_node("NAUO3", &n1)) {
        return;
    }

    if (!window_get_required_node("NAUO2.002", &n2)) {
        return;
    }

    gl_mat tr1, tr2;
    scene_node_get_local_tr(active_scene, &n1, tr1.data);
    scene_node_get_local_tr(active_scene, &n2, tr2.data);

    gl_vec3 cur_tr1 = gl_mat_get_translation(tr1);
    gl_vec3 cur_tr2 = gl_mat_get_translation(tr2);

    if (hold) {
        cur_tr1 = gl_vec3_add(cur_tr1, gl_vec3_new(0, 0, speed * dt));
        cur_tr2 = gl_vec3_add(cur_tr2, gl_vec3_new(0, 0, -speed * dt));
    }

    if (release) {
        cur_tr1 = gl_vec3_add(cur_tr1, gl_vec3_new(0, 0, -speed * dt));
        cur_tr2 = gl_vec3_add(cur_tr2, gl_vec3_new(0, 0, speed * dt));
    }

    if (hold || release) {
        cur_tr1.z = gl_clamp(cur_tr1.z, tight - relax_delta, tight);
        cur_tr2.z = gl_clamp(cur_tr2.z, tight, tight + relax_delta);

        tr1 = gl_mat_set_translation(tr1, cur_tr1);
        tr2 = gl_mat_set_translation(tr2, cur_tr2);

        scene_node_set_local_tr(active_scene, &n1, tr1.data);
        scene_node_set_local_tr(active_scene, &n2, tr2.data);

        window_scene_views_mark_as_dirty();
    }
}

void window_init(struct window_config const* config) {
    device_init(3, 3);
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

void window_run() {
    while(device_window_valid()) {

        device_update_events();

        gfx_begin_pass(0,
                       GFX_PASS_OPTION_DEPTH_TEST,
                       GFX_PASS_ACTION_CLEAR_COLOR | GFX_PASS_ACTION_CLEAR_DEPTH,
                       bg_color);

        gui_begin_frame();

        window_scene_view_draw();
        window_manipulator_demo();

        /*
         * Developer tools are useful while debugging, but they distract
         * during the final project demonstration.
         */
        if (show_dev_tools) {
            window_shader_editor();
            window_log();
        }

        int32_t width, height;
        device_window_dimensions_get(&width, &height);
        gfx_viewport_set(width, height);

        gui_end_frame();
        gfx_end_pass();
        device_refresh();
    }
}

void window_finalize(){
    scene_view_destroy(views[0]);
    scene_delete(active_scene);
    gui_finalize();
    gfx_terminate();
    device_delete(device);
    device_terminate();
}
