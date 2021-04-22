cmake_minimum_required(VERSION 3.17)
project(Ibc C)

set(SRC main.c)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

include_directories(Engine)
include_directories(Core)

add_executable(${PROJECT_NAME} ${SRC})
add_subdirectory(Engine)
add_subdirectory(Core)
add_subdirectory(Game)
add_subdirectory(Tools)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine Core)