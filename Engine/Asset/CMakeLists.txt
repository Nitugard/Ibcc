set(PROJECT_NAME Asset)

set(ASSET_SRC
        ./Texture/Texture.c
        ./Json/Json.c
        ./Model/Model.c
        ./Asset.c
        )

add_library(${PROJECT_NAME} SHARED ${ASSET_SRC})

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/bin/)
target_link_libraries(${PROJECT_NAME} PRIVATE Core)

