set(PROJECT_NAME Json)

add_library(${PROJECT_NAME} SHARED Json.c)

target_include_directories(${PROJECT_NAME} PRIVATE ../ThirdParty/jsmn/)
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/bin/)
