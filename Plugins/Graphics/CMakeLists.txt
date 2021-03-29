set(PROJECT_NAME Graphics)

add_library(${PROJECT_NAME} SHARED Graphics.c)

target_include_directories(${PROJECT_NAME} PRIVATE )
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/bin/)
target_link_libraries(${PROJECT_NAME} PRIVATE Core -lopengl32 gl3w)
