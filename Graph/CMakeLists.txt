cmake_minimum_required(VERSION 3.3)
set(PROJECT_NAME Graph)

add_library(${PROJECT_NAME} ./Scene/Scene.c ./Interface/Text.c ./Draw/Draw.c ./Interface/Sprite.c)

target_include_directories(${PROJECT_NAME} PRIVATE ./ThirdParty/include/)
target_link_directories(${PROJECT_NAME} PRIVATE ./ThirdParty/bin/)

target_link_libraries(${PROJECT_NAME} PRIVATE Core Engine)
