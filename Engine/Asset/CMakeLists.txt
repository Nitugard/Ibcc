set(PROJECT_NAME Asset)

add_library(${PROJECT_NAME} SHARED Asset.c AssetGlue.c Json/Json.c Shader/Shader.c)

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/bin/)
target_link_libraries(${PROJECT_NAME} PRIVATE Core)

