set(PROJECT_NAME Device)

add_library(${PROJECT_NAME} SHARED Device.c)

target_include_directories(${PROJECT_NAME} PRIVATE
        ../ThirdParty/glfw/include/
        )
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/bin/)
target_link_libraries(${PROJECT_NAME} PRIVATE Core glfw)
