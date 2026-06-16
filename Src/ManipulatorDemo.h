/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef MANIPULATOR_DEMO_H
#define MANIPULATOR_DEMO_H

#include <stdbool.h>
#include "Scene.h"

#define MANIPULATOR_NODE_COUNT 5

typedef struct {
    const char* node_name;
    float       local_tr[16];
} manipulator_saved_tr;

typedef struct manipulator_demo {
    scene_handle scene;

    /* Resolved scene node handles */
    scene_node carriage_x;      /* NAUO2      — X translation rail   */
    scene_node lift_y;          /* NAUO2.001  — Y lift column         */
    scene_node wrist;           /* NAUO1.003  — rotary wrist joint    */
    scene_node finger_left;     /* NAUO3      — left gripper finger   */
    scene_node finger_right;    /* NAUO2.002  — right gripper finger  */

    /* Motion targets */
    float x_target;             /* ±X_RAIL_EXTENT                     */
    float lift_target;          /* Y height goal                       */
    float wrist_angle;          /* accumulated Y-rotation in degrees  */

    /* Gripper commands (set each frame by UI or autoplay) */
    bool  close_gripper;
    bool  open_gripper;

    /* -- Internal state -- */
    float wrist_rot_dir;        /* manual CW/CCW: -1.0, 0.0, +1.0    */
    manipulator_saved_tr saved[MANIPULATOR_NODE_COUNT];
    bool  transforms_captured;

    /* Autoplay sequencer */
    bool  autoplay;
    int   phase;
    float phase_time;
    bool  has_rot_target;
    float rot_target;
} manipulator_demo;

/* Resolve nodes from scene; returns false if any node is missing. */
bool manipulator_demo_init(manipulator_demo* demo, scene_handle scene);

/* Advance simulation one frame. Returns true if any node transform changed. */
bool manipulator_demo_update(manipulator_demo* demo, float dt);

/* Draw ImGui controls panel section (call inside an igBegin/End block). */
void manipulator_demo_draw_ui(manipulator_demo* demo);

/* Restore all nodes to initial transforms and reset all state. */
void manipulator_demo_reset(manipulator_demo* demo);

#endif /* MANIPULATOR_DEMO_H */
