set(PROJECT_NAME Shader)

add_library(${PROJECT_NAME} SHARED Shader.c)

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/bin/)
target_link_libraries(${PROJECT_NAME} PRIVATE Core)

