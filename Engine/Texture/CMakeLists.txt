set(PROJECT_NAME Texture)

add_library(${PROJECT_NAME} SHARED Texture.c)

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/bin/)
target_link_libraries(${PROJECT_NAME} PRIVATE Asset Core)

