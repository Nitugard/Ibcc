cmake_minimum_required(VERSION 3.3)
set(PROJECT_NAME Engine)

add_library(${PROJECT_NAME} ./ThirdParty/src/gl3w.c)
find_library(glfw3_lib glfw3 ./ThirdParty/lib/)

add_subdirectory(Graphics)
add_subdirectory(Device)
add_subdirectory(Model)
add_subdirectory(Font)
add_subdirectory(Texture)

target_include_directories(${PROJECT_NAME} PRIVATE ./ThirdParty/include/)
target_link_directories(${PROJECT_NAME} PRIVATE ./ThirdParty/bin/)

target_link_libraries(${PROJECT_NAME} PRIVATE Core ${glfw3_lib})

