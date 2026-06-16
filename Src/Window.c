/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

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
#include "ManipulatorDemo.h"

#define MAXIMUM_WINDOW_LOGS 1024
#define MAXIMUM_SKYBOX_OPTIONS 32
#define DEFAULT_SKYBOX_PATH "./Data/Default.hdr"

typedef struct window_log_data{
    char* log;
    time_t time;
    int32_t length;
} window_log_data;

window_log_data logs[MAXIMUM_WINDOW_LOGS];
int32_t logs_count;

scene_view_handle views[2];

scene_handle active_scene;
mdl_handle model;

/* Selected node for object/material inspector (-1 = none) */
static scene_node selected_node;
static bool      selected_node_valid = false;
device_handle device;

typedef struct {
    const char* project_name;
    const char* student_name;
    const char* professor_name;
    const char* course_name;
    const char* faculty_name;
} window_project_info;

static window_project_info project_info = {
    .project_name  = "Grafi\xc4\x8dki prikaz manipulatora",   /* Grafički */
    .student_name  = "Dragutin Sredojevi\xc4\x87",            /* Sredojević */
    .professor_name = "Du\xc5\xa1" "an Nedeljkovi\xc4\x87", /* Dušan Nedeljković */
    .course_name   = "Kompjuterska grafika",
    .faculty_name  = "Ma\xc5\xa1inski fakultet",              /* Mašinski */
};

static float frame_dt = 0.0f;

float bg_color[4] = {0.09f, 0.09f, 0.12f, 1.0f};

typedef struct skybox_option {
    char label[128];
    char path[MAXIMUM_PATH_LENGTH];
} skybox_option;

static skybox_option skybox_options[MAXIMUM_SKYBOX_OPTIONS];
static const char* skybox_option_labels[MAXIMUM_SKYBOX_OPTIONS];
static int32_t skybox_options_count = 0;
static int32_t selected_skybox = 0;

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

static bool window_skybox_option_exists(const char* path)
{
    for (int32_t i = 0; i < skybox_options_count; ++i) {
        if (strcmp(skybox_options[i].path, path) == 0) {
            return true;
        }
    }

    return false;
}

static void window_skybox_label_from_file(char* dst, int32_t dst_size, const char* file_name)
{
    snprintf(dst, dst_size, "%s", file_name);
    char* ext = strrchr(dst, '.');
    if (ext != 0) {
        *ext = '\0';
    }
}

static void window_skybox_options_add(const char* label, const char* path)
{
    if (skybox_options_count >= MAXIMUM_SKYBOX_OPTIONS || window_skybox_option_exists(path)) {
        return;
    }

    snprintf(skybox_options[skybox_options_count].label,
             sizeof(skybox_options[skybox_options_count].label),
             "%s",
             label);
    snprintf(skybox_options[skybox_options_count].path,
             sizeof(skybox_options[skybox_options_count].path),
             "%s",
             path);
    skybox_option_labels[skybox_options_count] = skybox_options[skybox_options_count].label;
    skybox_options_count += 1;
}

static void window_skybox_options_scan_hdr_folder(const char* folder)
{
#ifdef _WIN32
    char pattern[MAXIMUM_PATH_LENGTH];
    snprintf(pattern, sizeof(pattern), "%s*.hdr", folder);

    WIN32_FIND_DATAA data;
    HANDLE find = FindFirstFileA(pattern, &data);
    if (find == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            char path[MAXIMUM_PATH_LENGTH];
            char label[128];
            snprintf(path, sizeof(path), "%s%s", folder, data.cFileName);
            window_skybox_label_from_file(label, sizeof(label), data.cFileName);
            window_skybox_options_add(label, path);
        }
    } while (FindNextFileA(find, &data));

    FindClose(find);
#else
    (void)folder;
#endif
}

static void window_skybox_options_scan_cubemap_folders(void)
{
#ifdef _WIN32
    WIN32_FIND_DATAA data;
    HANDLE find = FindFirstFileA("./Data/skyboxes/*", &data);
    if (find == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
            strcmp(data.cFileName, ".") != 0 &&
            strcmp(data.cFileName, "..") != 0) {
            char path[MAXIMUM_PATH_LENGTH];
            snprintf(path, sizeof(path), "./Data/skyboxes/%s/", data.cFileName);
            window_skybox_options_add(data.cFileName, path);
        }
    } while (FindNextFileA(find, &data));

    FindClose(find);
#endif
}

static void window_skybox_options_select_path(const char* path)
{
    selected_skybox = 0;
    for (int32_t i = 0; i < skybox_options_count; ++i) {
        if (strcmp(skybox_options[i].path, path) == 0) {
            selected_skybox = i;
            return;
        }
    }
}

static void window_skybox_options_refresh(void)
{
    skybox_options_count = 0;
    selected_skybox = 0;

    window_skybox_options_scan_hdr_folder("./Data/");
    window_skybox_options_scan_hdr_folder("./Data/hdri/");
    window_skybox_options_scan_cubemap_folders();

    if (skybox_options_count == 0) {
        window_skybox_options_add("Default", DEFAULT_SKYBOX_PATH);
    }

    window_skybox_options_select_path(DEFAULT_SKYBOX_PATH);
}

void window_scene_views_mark_as_dirty(){
    if (views[0]) scene_view_flag_dirty(views[0]);
}

void window_scene_view_create(){
    views[0] = scene_view_create(32, 32, SCENE_VIEW_PERSPECTIVE);
    views[1] = NULL;
}

static void window_view_options(void)
{
    if (skybox_options_count > 0) {
        int current_skybox = selected_skybox;
        if (igCombo_Str_arr("Okruzenje", &current_skybox, skybox_option_labels, skybox_options_count, 8)) {
            selected_skybox = current_skybox;
            scene_set_skybox_path(active_scene, skybox_options[selected_skybox].path);
            scene_view_flag_dirty(views[0]);
        }
    }

    float fov = scene_view_get_fov(views[0]);
    if (igSliderFloat("Vidno polje", &fov, 10.0f, 120.0f, "%.1f", 0))
        scene_view_set_fov(views[0], fov);

    scene_view_type view_type = scene_view_get_type(views[0]);
    if (igRadioButton_Bool("Prednja", view_type == SCENE_VIEW_ORTOGRAPHIC_FRONT))
        scene_view_set_type(views[0], SCENE_VIEW_ORTOGRAPHIC_FRONT);
    igSameLine(0.0f, -1.0f);
    if (igRadioButton_Bool("Bocna", view_type == SCENE_VIEW_ORTOGRAPHIC_SIDE))
        scene_view_set_type(views[0], SCENE_VIEW_ORTOGRAPHIC_SIDE);
    igSameLine(0.0f, -1.0f);
    if (igRadioButton_Bool("Perspektiva", view_type == SCENE_VIEW_PERSPECTIVE))
        scene_view_set_type(views[0], SCENE_VIEW_PERSPECTIVE);

    bool show_gizmos = scene_view_get_gizmos_visible(views[0]);
    if (igCheckbox("Prikazi ose", &show_gizmos))
        scene_view_set_gizmos_visible(views[0], show_gizmos);

    bool show_skybox = scene_get_skybox_render(active_scene);
    if (igCheckbox("Prikazi okruzenje", &show_skybox)) {
        scene_set_skybox_render(active_scene, show_skybox);
        scene_view_flag_dirty(views[0]);
    }

    bool show_plane = scene_get_plane_render(active_scene);
    if (igCheckbox("Prikazi ravan", &show_plane)) {
        scene_set_plane_render(active_scene, show_plane);
        scene_view_flag_dirty(views[0]);
    }

    float skybox_exposure = scene_get_skybox_exposure(active_scene);
    if (igSliderFloat("HDR ekspozicija", &skybox_exposure, 0.1f, 5.0f, "%.2f", 0)) {
        scene_set_skybox_exposure(active_scene, skybox_exposure);
        scene_view_flag_dirty(views[0]);
    }

    bool wireframe = scene_view_get_wireframe(views[0]);
    if (igCheckbox("Zicani prikaz", &wireframe))
        scene_view_set_wireframe(views[0], wireframe);
}

static void window_scene_view_overlay(void);
static void window_object_inspector(void);
static void window_material_inspector(void);

static manipulator_demo demo;

void window_scene_view_draw(){
    int32_t ww, wh;
    device_window_dimensions_get(&ww, &wh);

    /* Right panel: wide enough for text + buttons, but bounded on large displays. */
    float panel_w = gl_clamp((float)ww * 0.23f, 260.0f, 380.0f);
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

            window_scene_view_overlay();
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

        igBeginChild_Str("PanelScroll", (ImVec2){0.0f, 0.0f}, false, 0);
        {
            /* ---- Project identity ---- */
            igSpacing();
            igTextColored((ImVec4){0.50f, 0.78f, 1.00f, 1.00f}, "%s", project_info.course_name);
            igTextColored((ImVec4){0.70f, 0.70f, 0.76f, 1.00f}, "%s", project_info.faculty_name);
            igSpacing();
            igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){1.0f, 1.0f, 1.0f, 1.0f});
            igTextWrapped("%s", project_info.project_name);
            igPopStyleColor(1);
            igSpacing();
            igText("Student: %s", project_info.student_name);
            igText("Profesor: %s", project_info.professor_name);
            igSeparator();

            /* ---- Camera ---- */
            igTextDisabled("KAMERA");
            window_view_options();
            igSeparator();

            /* ---- Manipulator controls ---- */
            igTextDisabled("MANIPULATOR");
            igSpacing();
            manipulator_demo_draw_ui(&demo);
            igSeparator();

            /* ---- Object inspector ---- */
            window_object_inspector();
            window_material_inspector();
            igSeparator();

            /* ---- Camera control hints ---- */
            igTextDisabled("KONTROLE");
            igText("Desni klik     Rotacija");
            igText("Srednji klik   Pomeranje");
            igText("Tockic         Zumiranje");
            igSeparator();

            /* ---- Stats ---- */
            igTextDisabled("STATISTIKA");
            {
                igText("FPS: %.0f", frame_dt > 0.00001f ? 1.0f / frame_dt : 0.0f);
            }
            igSeparator();
        }
        igEndChild();

        igEnd();
    }
}

static void window_scene_view_overlay(void)
{
}

/* ---- Object inspector helpers ---- */

static void window_node_tree_recursive(scene_node* parent, int depth) {
    int32_t child_count = 0;
    scene_node_children_count(active_scene, parent, &child_count);

    ImGuiTreeNodeFlags_ flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                ImGuiTreeNodeFlags_SpanAvailWidth;
    if (child_count == 0)
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    bool is_selected = selected_node_valid && selected_node.internal == parent->internal;
    if (is_selected)
        flags |= ImGuiTreeNodeFlags_Selected;

    const char* label = (parent->name && parent->name[0]) ? parent->name : "(bez naziva)";
    bool open = igTreeNodeEx_Str(label, flags);

    if (igIsItemClicked(0)) {
        if (is_selected) {
            /* Deselect on second click */
            selected_node_valid = false;
            scene_set_selected_node(active_scene, NULL);
        } else {
            selected_node = *parent;
            selected_node_valid = true;
            scene_set_selected_node(active_scene, &selected_node);
        }
        scene_view_flag_dirty(views[0]);
    }

    if (open && child_count > 0) {
        for (int32_t i = 0; i < child_count; ++i) {
            scene_node child = {0};
            scene_node_children_get_at(active_scene, i, parent, &child);
            window_node_tree_recursive(&child, depth + 1);
        }
        igTreePop();
    }
}

static void window_object_inspector(void) {
    igTextDisabled("OBJEKTI");
    igSpacing();

    int32_t root_count = 0;
    scene_node_root_count(active_scene, &root_count);

    for (int32_t i = 0; i < root_count; ++i) {
        scene_node root = {0};
        scene_node_root_get_at(active_scene, i, &root);
        window_node_tree_recursive(&root, 0);
    }
}

static void window_material_inspector(void) {
    if (!selected_node_valid) return;

    int32_t mat_count = scene_node_get_material_count(active_scene, &selected_node);
    if (mat_count <= 0) return;

    igSeparator();
    igTextDisabled("MATERIJAL");
    igSpacing();

    for (int32_t i = 0; i < mat_count; ++i) {
        float color[4]   = {1.0f, 1.0f, 1.0f, 1.0f};
        float metallic   = 0.0f;
        float roughness  = 0.5f;
        scene_node_get_material(active_scene, &selected_node, i, color, &metallic, &roughness);

        char label[64];
        if (mat_count > 1)
            snprintf(label, sizeof(label), "Primitiv %d", i + 1);
        else
            snprintf(label, sizeof(label), "Boja");

        bool changed = false;
        igPushID_Int(i);

        changed |= igColorEdit4(label, color,
            ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_PickerHueWheel);
        changed |= igSliderFloat("Metalnost",  &metallic,  0.0f, 1.0f, "%.2f", 0);
        changed |= igSliderFloat("Hrapavost",  &roughness, 0.0f, 1.0f, "%.2f", 0);

        if (changed) {
            scene_node_set_material(active_scene, &selected_node, i, color, metallic, roughness);
            scene_view_flag_dirty(views[0]);
        }

        igPopID();
        if (i + 1 < mat_count) igSpacing();
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


void window_init(struct window_config const* config) {
    device_init(3, 3);
    device = device_new(config->title, config->width, config->height, config->vsync, config->fullscreen,
                        config->resizable);
    device_set_current(device);
    gfx_init();
    gui_init();

    gfx_log_callback_set(window_on_gfx_log);
    window_skybox_options_refresh();

    model = mdl_load("./Data/Manipulator.gltf");
    scene_desc desc = {
            .skybox = {
                    .path = DEFAULT_SKYBOX_PATH,
                    .render = true,
            },
            .model = model,
    };

    active_scene = scene_new(&desc);
    mdl_unload(model);

    manipulator_demo_init(&demo, active_scene);
    scene_shadow_pass(active_scene);

    window_scene_view_create();
    os_memset(logs, 0, sizeof(logs));
    logs_count = 0;
}

void window_run() {
    while (device_window_valid()) {

        device_update_events();

        gfx_begin_pass(0,
                       GFX_PASS_OPTION_DEPTH_TEST,
                       GFX_PASS_ACTION_CLEAR_COLOR | GFX_PASS_ACTION_CLEAR_DEPTH,
                       bg_color);

        gui_begin_frame();

        frame_dt = (float)device_dt_get();
        bool scene_changed = manipulator_demo_update(&demo, frame_dt);
        if (scene_changed) {
            scene_shadow_pass(active_scene);
            window_scene_views_mark_as_dirty();
        }

        window_scene_view_draw();

#ifndef NDEBUG
        //window_log();
#endif

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
