cmake_minimum_required(VERSION 3.17)
project(Ibcc C)

set(SRC main.c)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-o3 -Wall")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Build/Bin)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

include_directories(Engine)
include_directories(Core)
include_directories(Graph)

add_executable(${PROJECT_NAME} ${SRC})

add_subdirectory(Core)
add_subdirectory(Engine)
add_subdirectory(Graph)
add_subdirectory(Tools)

target_link_libraries(${PROJECT_NAME} PRIVATE Graph Engine Core)