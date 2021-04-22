cmake_minimum_required(VERSION 3.3)
set(PROJECT_NAME Engine)


add_library(${PROJECT_NAME} )
target_include_directories(${PROJECT_NAME} PRIVATE
        ./ThirdParty/glfw/include/
        ./ThirdParty/gl3w/include/
        )

add_subdirectory(ThirdParty/glfw/)
add_subdirectory(Graphics)
add_subdirectory(Device)
add_subdirectory(Asset)
add_subdirectory(Scene)

target_link_libraries(${PROJECT_NAME} PUBLIC Core glfw)

#find_library(SF_LIB ../Engine/SoftFloat HINTS ${CMAKE_CURRENT_BINARY_DIR})
#target_link_libraries(${PROJECT_NAME} PUBLIC ${SF_LIB})
