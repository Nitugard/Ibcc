set(PROJECT_NAME Model)

add_library(${PROJECT_NAME} SHARED Model.c)

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/bin/)
target_link_libraries(${PROJECT_NAME} PRIVATE Core Texture Asset)
