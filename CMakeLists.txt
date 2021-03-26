cmake_minimum_required(VERSION 3.17)
project(FixedPhysics C)

set(SRC
        Core/FixedMath/FixedMath.c
        Core/Device/DeviceGlfw.c
        Core/Graphics/GraphicsGL.c
        )

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(THIRD_PARTY_SRC Core/ThirdParty/gl3w/src/gl3w.c)


add_executable(${PROJECT_NAME} main.c ${SRC} ${THIRD_PARTY_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
        Core/ThirdParty/gl3w/include/
        Core/ThirdParty/glfw3/include/
        )
target_link_directories(${PROJECT_NAME} PUBLIC
        Core/ThirdParty/libs/
        )
target_link_libraries(${PROJECT_NAME} PRIVATE -lopengl32 -lglfw3)
