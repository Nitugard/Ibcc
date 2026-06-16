/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Gui.h"

#include <stdio.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include <cimgui.h>
#include <cimgui_impl.h>
#include "Device.h"

struct ImGuiContext* ctx;
struct ImGuiIO* io;

void gui_init() {
    ctx = igCreateContext(NULL);
    io = igGetIO();
    io->IniFilename = NULL;

    const char *glsl_version = "#version 330 core";
    ImGui_ImplGlfw_InitForOpenGL(device_window_handle(), true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    igStyleColorsDark(NULL);

    /* ---- Layout metrics ---- */
    ImGuiStyle* s = igGetStyle();
    s->WindowRounding    = 0.0f;   /* sharp panel edges look more professional */
    s->FrameRounding     = 4.0f;
    s->GrabRounding      = 4.0f;
    s->ScrollbarRounding = 4.0f;
    s->PopupRounding     = 4.0f;
    s->WindowBorderSize  = 1.0f;
    s->FrameBorderSize   = 0.0f;
    s->WindowPadding     = (ImVec2){14.0f, 14.0f};
    s->FramePadding      = (ImVec2){ 8.0f,  5.0f};
    s->ItemSpacing       = (ImVec2){ 8.0f,  7.0f};
    s->ItemInnerSpacing  = (ImVec2){ 6.0f,  6.0f};
    s->ScrollbarSize     = 10.0f;
    s->GrabMinSize       = 10.0f;

    /* ---- Colour palette: dark blue-grey, professional ---- */
    ImVec4* c = s->Colors;
    c[ImGuiCol_WindowBg]             = (ImVec4){0.10f, 0.10f, 0.13f, 1.00f};
    c[ImGuiCol_ChildBg]              = (ImVec4){0.10f, 0.10f, 0.13f, 1.00f};
    c[ImGuiCol_PopupBg]              = (ImVec4){0.10f, 0.10f, 0.13f, 1.00f};
    c[ImGuiCol_Border]               = (ImVec4){0.22f, 0.22f, 0.28f, 1.00f};
    c[ImGuiCol_FrameBg]              = (ImVec4){0.16f, 0.16f, 0.20f, 1.00f};
    c[ImGuiCol_FrameBgHovered]       = (ImVec4){0.22f, 0.22f, 0.28f, 1.00f};
    c[ImGuiCol_FrameBgActive]        = (ImVec4){0.18f, 0.34f, 0.52f, 1.00f};
    c[ImGuiCol_TitleBg]              = (ImVec4){0.08f, 0.08f, 0.10f, 1.00f};
    c[ImGuiCol_TitleBgActive]        = (ImVec4){0.08f, 0.08f, 0.10f, 1.00f};
    c[ImGuiCol_TitleBgCollapsed]     = (ImVec4){0.08f, 0.08f, 0.10f, 1.00f};
    c[ImGuiCol_ScrollbarBg]          = (ImVec4){0.08f, 0.08f, 0.10f, 0.00f};
    c[ImGuiCol_ScrollbarGrab]        = (ImVec4){0.26f, 0.26f, 0.34f, 1.00f};
    c[ImGuiCol_ScrollbarGrabHovered] = (ImVec4){0.32f, 0.32f, 0.42f, 1.00f};
    c[ImGuiCol_ScrollbarGrabActive]  = (ImVec4){0.22f, 0.46f, 0.70f, 1.00f};
    c[ImGuiCol_CheckMark]            = (ImVec4){0.34f, 0.60f, 0.90f, 1.00f};
    c[ImGuiCol_SliderGrab]           = (ImVec4){0.30f, 0.56f, 0.86f, 1.00f};
    c[ImGuiCol_SliderGrabActive]     = (ImVec4){0.40f, 0.66f, 0.96f, 1.00f};
    c[ImGuiCol_Button]               = (ImVec4){0.20f, 0.22f, 0.28f, 1.00f};
    c[ImGuiCol_ButtonHovered]        = (ImVec4){0.24f, 0.44f, 0.66f, 1.00f};
    c[ImGuiCol_ButtonActive]         = (ImVec4){0.18f, 0.36f, 0.56f, 1.00f};
    c[ImGuiCol_Header]               = (ImVec4){0.20f, 0.36f, 0.54f, 1.00f};
    c[ImGuiCol_HeaderHovered]        = (ImVec4){0.26f, 0.44f, 0.64f, 1.00f};
    c[ImGuiCol_HeaderActive]         = (ImVec4){0.18f, 0.32f, 0.50f, 1.00f};
    c[ImGuiCol_Separator]            = (ImVec4){0.20f, 0.20f, 0.26f, 1.00f};
    c[ImGuiCol_SeparatorHovered]     = (ImVec4){0.28f, 0.28f, 0.36f, 1.00f};
    c[ImGuiCol_Tab]                  = (ImVec4){0.12f, 0.12f, 0.16f, 1.00f};
    c[ImGuiCol_TabHovered]           = (ImVec4){0.26f, 0.44f, 0.64f, 1.00f};
    c[ImGuiCol_TabActive]            = (ImVec4){0.18f, 0.32f, 0.50f, 1.00f};
    c[ImGuiCol_TabUnfocused]         = (ImVec4){0.12f, 0.12f, 0.16f, 1.00f};
    c[ImGuiCol_TabUnfocusedActive]   = (ImVec4){0.16f, 0.26f, 0.40f, 1.00f};
    c[ImGuiCol_TextDisabled]         = (ImVec4){0.44f, 0.50f, 0.60f, 1.00f};
    c[ImGuiCol_Text]                 = (ImVec4){0.92f, 0.92f, 0.96f, 1.00f};
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
