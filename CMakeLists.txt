cmake_minimum_required(VERSION 3.24)
project(IbcWeb LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Bin)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

option(IBC_FETCH_DEPS "Download missing third-party dependencies during CMake configure." ON)

set(IBC_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Src/ThirdParty")
set(IBC_LOCAL_GL3W_SOURCE "${IBC_THIRD_PARTY_DIR}/src/gl3w.c")
set(IBC_LOCAL_GL3W_INCLUDE_DIR "${IBC_THIRD_PARTY_DIR}/include")
set(IBC_USE_VENDORED_DEPS OFF)

if(EXISTS "${IBC_THIRD_PARTY_DIR}/cimgui/cimgui.cpp"
        AND (EXISTS "${IBC_THIRD_PARTY_DIR}/gl3w.c" OR EXISTS "${IBC_LOCAL_GL3W_SOURCE}")
        AND EXISTS "${IBC_THIRD_PARTY_DIR}/cgltf.h"
        AND EXISTS "${IBC_THIRD_PARTY_DIR}/stbi_image.h")
    set(IBC_USE_VENDORED_DEPS ON)
endif()

if(IBC_USE_VENDORED_DEPS)
    set(CIMGUI_DIR "${IBC_THIRD_PARTY_DIR}/cimgui")
    if(EXISTS "${IBC_LOCAL_GL3W_SOURCE}")
        set(GL3W_SOURCE "${IBC_LOCAL_GL3W_SOURCE}")
        set(GL3W_INCLUDE_DIR "${IBC_LOCAL_GL3W_INCLUDE_DIR}")
    else()
        set(GL3W_SOURCE "${IBC_THIRD_PARTY_DIR}/gl3w.c")
        set(GL3W_INCLUDE_DIR "${IBC_THIRD_PARTY_DIR}")
    endif()
    set(CGLTF_INCLUDE_DIR "${IBC_THIRD_PARTY_DIR}")
    set(STB_INCLUDE_DIR "${IBC_THIRD_PARTY_DIR}")

    find_library(glfw3_lib NAMES glfw3 glfw
            PATHS "${IBC_THIRD_PARTY_DIR}" "${IBC_THIRD_PARTY_DIR}/glfw/lib"
            NO_DEFAULT_PATH)
    if(NOT glfw3_lib)
        find_package(glfw3 CONFIG QUIET)
    endif()
elseif(IBC_FETCH_DEPS)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4)
    FetchContent_Declare(
            cimgui
            GIT_REPOSITORY https://github.com/cimgui/cimgui.git
            GIT_TAG 1.88
            GIT_SUBMODULES imgui
            GIT_SUBMODULES_RECURSE TRUE
            SOURCE_SUBDIR cmake-do-not-add)
    FetchContent_Declare(
            gl3w
            GIT_REPOSITORY https://github.com/skaslev/gl3w.git
            GIT_TAG master
            SOURCE_SUBDIR cmake-do-not-add)
    FetchContent_Declare(
            cgltf
            GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
            GIT_TAG master
            SOURCE_SUBDIR cmake-do-not-add)
    FetchContent_Declare(
            stb
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG master
            SOURCE_SUBDIR cmake-do-not-add)

    FetchContent_MakeAvailable(glfw cimgui gl3w cgltf stb)

    set(CIMGUI_DIR "${cimgui_SOURCE_DIR}")
    if(EXISTS "${IBC_LOCAL_GL3W_SOURCE}")
        set(GL3W_SOURCE "${IBC_LOCAL_GL3W_SOURCE}")
        set(GL3W_INCLUDE_DIR "${IBC_LOCAL_GL3W_INCLUDE_DIR}")
    elseif(EXISTS "${gl3w_SOURCE_DIR}/src/gl3w.c")
        set(GL3W_SOURCE "${gl3w_SOURCE_DIR}/src/gl3w.c")
        set(GL3W_INCLUDE_DIR "${gl3w_SOURCE_DIR}/include")
    else()
        message(FATAL_ERROR
                "gl3w.c was not found. Generate it with gl3w_gen.py --root Src/ThirdParty "
                "or provide a vendored gl3w.c.")
    endif()
    set(CGLTF_INCLUDE_DIR "${cgltf_SOURCE_DIR}")
    set(STB_INCLUDE_DIR "${stb_SOURCE_DIR}")
else()
    message(FATAL_ERROR
            "Missing third-party dependencies. Provide Src/ThirdParty with cimgui, gl3w.c, "
            "cgltf.h, stbi_image.h, and GLFW, or configure with -DIBC_FETCH_DEPS=ON.")
endif()

add_library(cimgui STATIC
        "${CIMGUI_DIR}/cimgui.cpp"
        "${CIMGUI_DIR}/cimgui.h"
        "${CIMGUI_DIR}/imgui/imgui.cpp"
        "${CIMGUI_DIR}/imgui/imgui.h"
        "${CIMGUI_DIR}/imgui/imgui_widgets.cpp"
        "${CIMGUI_DIR}/imgui/imgui_draw.cpp"
        "${CIMGUI_DIR}/imgui/imgui_tables.cpp"
        "${CIMGUI_DIR}/imgui/imgui_demo.cpp"
        "${CIMGUI_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
        "${CIMGUI_DIR}/imgui/backends/imgui_impl_glfw.cpp")

target_include_directories(cimgui
        PUBLIC
        "${CIMGUI_DIR}"
        "${CIMGUI_DIR}/imgui"
        "${CIMGUI_DIR}/imgui/backends"
        "${CIMGUI_DIR}/generator/output"
        PRIVATE
        "${GL3W_INCLUDE_DIR}")
target_compile_definitions(cimgui PRIVATE
        IMGUI_IMPL_API=extern\ \"C\"
        IMGUI_IMPL_OPENGL_LOADER_GL3W)

if(TARGET glfw)
    target_link_libraries(cimgui PRIVATE glfw)
endif()

add_executable(IbcWeb
        main.c
        "${GL3W_SOURCE}"
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
        Src/ManipulatorDemo.c
        Src/ShadowRenderer.c
        Src/GroundRenderer.c
        Src/BrdfLut.c)

target_compile_options(IbcWeb PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-Wall>
        $<$<COMPILE_LANGUAGE:C>:-Wno-int-to-pointer-cast>)

target_include_directories(IbcWeb PRIVATE
        Src
        "${GL3W_INCLUDE_DIR}"
        "${CGLTF_INCLUDE_DIR}"
        "${STB_INCLUDE_DIR}")

target_link_libraries(IbcWeb PRIVATE cimgui)


if(TARGET glfw)
    target_link_libraries(IbcWeb PRIVATE glfw)
elseif(glfw3_lib)
    target_link_libraries(IbcWeb PRIVATE "${glfw3_lib}")
elseif(TARGET glfw3)
    target_link_libraries(IbcWeb PRIVATE glfw3)
else()
    message(FATAL_ERROR "GLFW was not found. Add it to Src/ThirdParty or enable IBC_FETCH_DEPS.")
endif()
