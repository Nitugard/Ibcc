/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Gui.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include "Device.h"

struct ImGuiContext* ctx;
struct ImGuiIO* io;

void gui_init() {
    ctx = igCreateContext(NULL);
    io = igGetIO();

    const char *glsl_version = "#version 330 core";
    ImGui_ImplGlfw_InitForOpenGL(device_window_handle(), true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    igStyleColorsDark(NULL);
}


void gui_finalize(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(ctx);
}

void gui_begin_frame(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}
void gui_end_frame(){
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}