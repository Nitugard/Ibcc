/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include "ManipulatorDemo.h"
#include "Scene.h"
#include "GlMath.h"

#include <stdio.h>
#include <string.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

/* ---- Motion constants ---- */
static const float X_RAIL_EXTENT = 1.38f;
static const float X_SPEED       = 1.00f;
static const float LIFT_SPEED    = 1.00f;
static const float LIFT_TOP      = 2.857f;
static const float LIFT_BOTTOM   = 2.408f;
static const float WRIST_SPEED   = 100.0f;
static const float WRIST_LIMIT   = 180.0f;
static const float GRIP_SPEED    = 0.05f;
static const float GRIP_CLOSED   = -0.031f;
static const float GRIP_DELTA    = 0.055f;

/* Ordered to match saved[] index → node pointer mapping in helpers */
static const char* const NODE_NAMES[MANIPULATOR_NODE_COUNT] = {
    "NAUO2",      /* [0] carriage_x   */
    "NAUO2.001",  /* [1] lift_y       */
    "NAUO1.003",  /* [2] wrist        */
    "NAUO3",      /* [3] finger_left  */
    "NAUO2.002",  /* [4] finger_right */
};

/* ---- Internal helpers ---- */

static bool resolve_node(scene_handle scene, const char* name, scene_node* out)
{
    if (!scene_node_get(scene, name, out)) {
        fprintf(stderr, "ManipulatorDemo: node '%s' not found\n", name);
        return false;
    }
    return true;
}

static scene_node* node_at(manipulator_demo* demo, int idx)
{
    switch (idx) {
        case 0: return &demo->carriage_x;
        case 1: return &demo->lift_y;
        case 2: return &demo->wrist;
        case 3: return &demo->finger_left;
        case 4: return &demo->finger_right;
    }
    return NULL;
}

/* ---- Public API ---- */

bool manipulator_demo_init(manipulator_demo* demo, scene_handle scene)
{
    memset(demo, 0, sizeof(*demo));
    demo->scene = scene;

    if (!resolve_node(scene, NODE_NAMES[0], &demo->carriage_x))  return false;
    if (!resolve_node(scene, NODE_NAMES[1], &demo->lift_y))       return false;
    if (!resolve_node(scene, NODE_NAMES[2], &demo->wrist))        return false;
    if (!resolve_node(scene, NODE_NAMES[3], &demo->finger_left))  return false;
    if (!resolve_node(scene, NODE_NAMES[4], &demo->finger_right)) return false;

    /* Capture initial local transforms for reset */
    for (int i = 0; i < MANIPULATOR_NODE_COUNT; ++i) {
        demo->saved[i].node_name = NODE_NAMES[i];
        scene_node_get_local_tr(scene, node_at(demo, i), demo->saved[i].local_tr);
    }
    demo->transforms_captured = true;

    demo->x_target    =  X_RAIL_EXTENT;
    demo->lift_target =  LIFT_TOP;
    demo->wrist_angle =  0.0f;

    return true;
}

void manipulator_demo_reset(manipulator_demo* demo)
{
    if (!demo->transforms_captured) return;

    for (int i = 0; i < MANIPULATOR_NODE_COUNT; ++i)
        scene_node_set_local_tr(demo->scene, node_at(demo, i), demo->saved[i].local_tr);

    demo->x_target       =  X_RAIL_EXTENT;
    demo->lift_target    =  LIFT_TOP;
    demo->wrist_angle    =  0.0f;
    demo->wrist_rot_dir  =  0.0f;
    demo->close_gripper  =  false;
    demo->open_gripper   =  false;
    demo->autoplay       =  false;
    demo->phase          =  0;
    demo->phase_time     =  0.0f;
    demo->has_rot_target =  false;
    demo->rot_target     =  0.0f;
}

/* ---- Autoplay sequencer ---- */

static void autoplay_advance(manipulator_demo* demo)
{
    demo->phase = (demo->phase + 1) % 6;
    demo->phase_time = 0.0f;
}

static void autoplay_update(manipulator_demo* demo, float dt)
{
    if (!demo->autoplay) {
        demo->has_rot_target = false;
        return;
    }

    demo->phase_time    += dt;
    demo->close_gripper  = false;
    demo->open_gripper   = false;
    demo->has_rot_target = false;

    switch (demo->phase) {
        case 0:
            demo->x_target = -X_RAIL_EXTENT;
            if (demo->phase_time >= 2.0f) autoplay_advance(demo);
            break;
        case 1:
            demo->lift_target = LIFT_BOTTOM;
            if (demo->phase_time >= 1.8f) autoplay_advance(demo);
            break;
        case 2:
            demo->has_rot_target = true;
            demo->rot_target     = WRIST_LIMIT;
            if (gl_abs(demo->wrist_angle - WRIST_LIMIT) <= 0.5f) autoplay_advance(demo);
            break;
        case 3:
            demo->close_gripper = true;
            if (demo->phase_time >= 1.2f) autoplay_advance(demo);
            break;
        case 4:
            demo->open_gripper = true;
            if (demo->phase_time >= 1.2f) autoplay_advance(demo);
            break;
        case 5:
            demo->x_target       = X_RAIL_EXTENT;
            demo->lift_target    = LIFT_TOP;
            demo->has_rot_target = true;
            demo->rot_target     = 0.0f;
            if (demo->phase_time >= 2.2f && gl_abs(demo->wrist_angle) <= 0.5f)
                autoplay_advance(demo);
            break;
    }
}

/* ---- Update ---- */

bool manipulator_demo_update(manipulator_demo* demo, float dt)
{
    const float eps = 0.0001f;
    bool dirty = false;

    autoplay_update(demo, dt);

    /* Carriage X */
    {
        gl_mat tr;
        scene_node_get_world_tr(demo->scene, &demo->carriage_x, tr.data);
        gl_vec3 pos   = gl_mat_get_translation(tr);
        float   delta = demo->x_target - pos.x;

        if (gl_abs(delta) > eps) {
            float dir   = delta / gl_abs(delta);
            float new_x = gl_clamp(pos.x + X_SPEED * dt * dir, -X_RAIL_EXTENT, X_RAIL_EXTENT);
            if (gl_abs(new_x - pos.x) > eps) {
                tr = gl_mat_set_translation(tr, gl_vec3_new(new_x, pos.y, pos.z));
                scene_node_set_world_tr(demo->scene, &demo->carriage_x, tr.data);
                dirty = true;
            }
        }
    }

    /* Lift Y */
    {
        gl_mat tr;
        scene_node_get_world_tr(demo->scene, &demo->lift_y, tr.data);
        gl_vec3 pos   = gl_mat_get_translation(tr);
        float   delta = demo->lift_target - pos.y;

        if (gl_abs(delta) > eps) {
            float dir   = delta / gl_abs(delta);
            float new_y = gl_clamp(pos.y + LIFT_SPEED * dt * dir, LIFT_BOTTOM, LIFT_TOP);
            if (gl_abs(new_y - pos.y) > eps) {
                tr = gl_mat_set_translation(tr, gl_vec3_new(pos.x, new_y, pos.z));
                scene_node_set_world_tr(demo->scene, &demo->lift_y, tr.data);
                dirty = true;
            }
        }
    }

    /* Wrist rotation */
    {
        gl_mat tr;
        scene_node_get_world_tr(demo->scene, &demo->wrist, tr.data);

        if (demo->has_rot_target) {
            float delta = demo->rot_target - demo->wrist_angle;
            if (gl_abs(delta) > eps) {
                float step = dt * WRIST_SPEED;
                demo->wrist_angle += gl_clamp(delta, -step, step);
                demo->wrist_angle  = gl_clamp(demo->wrist_angle, -WRIST_LIMIT, WRIST_LIMIT);
                gl_vec3 cur_pos = gl_mat_get_translation(tr);
                gl_mat  new_tr  = gl_mat_set_translation(gl_mat_rotate_y(demo->wrist_angle), cur_pos);
                scene_node_set_world_tr(demo->scene, &demo->wrist, new_tr.data);
                dirty = true;
            }
        } else if (gl_abs(demo->wrist_rot_dir) > eps) {
            demo->wrist_angle += dt * WRIST_SPEED * demo->wrist_rot_dir;
            demo->wrist_angle  = gl_clamp(demo->wrist_angle, -WRIST_LIMIT, WRIST_LIMIT);
            gl_vec3 cur_pos = gl_mat_get_translation(tr);
            gl_mat  new_tr  = gl_mat_set_translation(gl_mat_rotate_y(demo->wrist_angle), cur_pos);
            scene_node_set_world_tr(demo->scene, &demo->wrist, new_tr.data);
            dirty = true;
        }
    }

    /* Gripper fingers */
    if (demo->close_gripper || demo->open_gripper) {
        gl_mat tr1, tr2;
        scene_node_get_local_tr(demo->scene, &demo->finger_left,  tr1.data);
        scene_node_get_local_tr(demo->scene, &demo->finger_right, tr2.data);
        gl_vec3 p1 = gl_mat_get_translation(tr1);
        gl_vec3 p2 = gl_mat_get_translation(tr2);
        float   dz = GRIP_SPEED * dt;

        if (demo->close_gripper) {
            p1.z = gl_clamp(p1.z + dz, GRIP_CLOSED - GRIP_DELTA, GRIP_CLOSED);
            p2.z = gl_clamp(p2.z - dz, GRIP_CLOSED, GRIP_CLOSED + GRIP_DELTA);
        }
        if (demo->open_gripper) {
            p1.z = gl_clamp(p1.z - dz, GRIP_CLOSED - GRIP_DELTA, GRIP_CLOSED);
            p2.z = gl_clamp(p2.z + dz, GRIP_CLOSED, GRIP_CLOSED + GRIP_DELTA);
        }

        scene_node_set_local_tr(demo->scene, &demo->finger_left,
                                gl_mat_set_translation(tr1, p1).data);
        scene_node_set_local_tr(demo->scene, &demo->finger_right,
                                gl_mat_set_translation(tr2, p2).data);
        dirty = true;
    }

    return dirty;
}

/* ---- ImGui UI ---- */

void manipulator_demo_draw_ui(manipulator_demo* demo)
{
    ImVec2 avail;
    igGetContentRegionAvail(&avail);
    float hw = (avail.x - igGetStyle()->ItemSpacing.x) * 0.5f;

    if (igButton("Resetuj manipulator", (ImVec2){-1.0f, 0.0f}))
        manipulator_demo_reset(demo);

    if (igButton("Pokreni animaciju", (ImVec2){-1.0f, 0.0f})) {
        demo->autoplay   = true;
        demo->phase      = 0;
        demo->phase_time = 0.0f;
    }

    if (igButton("Pauziraj", (ImVec2){-1.0f, 0.0f})) {
        demo->autoplay      = false;
        demo->close_gripper = false;
        demo->open_gripper  = false;
        demo->wrist_rot_dir = 0.0f;
    }

    igSeparator();

    if (igButton("Pomak X", (ImVec2){-1.0f, 0.0f}))
        demo->x_target = -demo->x_target;

    bool rot_active = false;
    igButton("Desno", (ImVec2){hw, 0.0f});
    if (igIsItemActive()) { demo->wrist_rot_dir = -1.0f; rot_active = true; }
    igSameLine(0.0f, -1.0f);
    igButton("Levo", (ImVec2){hw, 0.0f});
    if (igIsItemActive()) { demo->wrist_rot_dir =  1.0f; rot_active = true; }
    if (!rot_active) demo->wrist_rot_dir = 0.0f;

    if (igButton("Pomak Y", (ImVec2){-1.0f, 0.0f})) {
        float mid = (LIFT_TOP + LIFT_BOTTOM) * 0.5f;
        demo->lift_target = (demo->lift_target > mid) ? LIFT_BOTTOM : LIFT_TOP;
    }

    igButton("Stisni",  (ImVec2){-1.0f, 0.0f}); demo->close_gripper = igIsItemActive();
    igButton("Otpusti", (ImVec2){-1.0f, 0.0f}); demo->open_gripper  = igIsItemActive();
}
