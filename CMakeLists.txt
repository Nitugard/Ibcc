cmake_minimum_required(VERSION 3.0)
project(IbcWeb)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "-o3 -Wall")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Bin)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(cimgui STATIC
        ./Src/ThirdParty/cimgui/cimgui.cpp
        ./Src/ThirdParty/cimgui/cimgui.h
        ./Src/ThirdParty/cimgui/imgui/imgui.cpp
        ./Src/ThirdParty/cimgui/imgui/imgui.h
        ./Src/ThirdParty/cimgui/imgui/imgui_widgets.cpp
        ./Src/ThirdParty/cimgui/imgui/imgui_draw.cpp
        ./Src/ThirdParty/cimgui/imgui/imgui_tables.cpp
        ./Src/ThirdParty/cimgui/imgui/imgui_demo.cpp
        ./Src/ThirdParty/cimgui/imgui/backends/imgui_impl_opengl3.cpp
        ./Src/ThirdParty/cimgui/imgui/backends/imgui_impl_glfw.cpp

        )
target_include_directories(cimgui INTERFACE cimgui)
target_include_directories(cimgui PRIVATE ./Src/ThirdParty/cimgui/imgui/ ./Src/ThirdParty/)
target_compile_definitions(cimgui PRIVATE
        IMGUI_IMPL_API=extern\ \"C\"
        IMGUI_IMPL_OPENGL_LOADER_GL3W)

add_executable(IbcWeb main.c
        ./Src/ThirdParty/gl3w.c
        Src/Skybox.c
        Src/Model.c
        Src/Scene.c
        Src/Allocator.c
        Src/Device.c
        Src/GlMath.c
        Src/Graphics.c
        Src/Controller.c
        Src/Gui.c
        Src/SceneView.c
        Src/Window.c
        Src/Wire.c
        )

find_library(glfw3_lib glfw3 ./Src/ThirdParty/)
target_link_libraries(${PROJECT_NAME} PRIVATE ${glfw3_lib} cimgui)

target_include_directories(${PROJECT_NAME} PRIVATE ./Src/ThirdParty ./Src/ThirdParty/cimgui)
target_link_directories(${PROJECT_NAME} PRIVATE ./Src/ThirdParty)
