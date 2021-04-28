cmake_minimum_required(VERSION 3.3)
set(PROJECT_NAME EmbedFile)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} Core)