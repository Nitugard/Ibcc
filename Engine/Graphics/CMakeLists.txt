set(PROJECT_NAME Graphics)

add_library(${PROJECT_NAME} SHARED Graphics.c ../Asset/Shader/Shader.c ../ThirdParty/gl3w/src/gl3w.c)
target_include_directories(${PROJECT_NAME} PRIVATE ../ThirdParty/gl3w/include)
target_link_libraries(${PROJECT_NAME} PRIVATE Core Asset)