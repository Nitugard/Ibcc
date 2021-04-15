set(PROJECT_NAME Core)

set(SRC Src/Containers.c Src/Os.c Src/SoftFloat.c)

add_library(${PROJECT_NAME} SHARED ${SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ./ThirdParty/softfloat/include)
find_library(SF_LIB softfloat)
target_link_libraries(${PROJECT_NAME} PUBLIC ${SF_LIB})
