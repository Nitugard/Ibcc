add_subdirectory(Game)
add_subdirectory(Graphics)
add_subdirectory(Device)
add_subdirectory(Asset)

add_subdirectory(ThirdParty/glfw/)

#find_library(SF_LIB ../Engine/SoftFloat HINTS ${CMAKE_CURRENT_BINARY_DIR})
#target_link_libraries(${PROJECT_NAME} PUBLIC ${SF_LIB})
