set(PROJECT_NAME Core)

set(SRC
        Containers/Array.c
        Containers/String.c
        Containers/Hash.c
        Os/Allocator.c
        Os/File.c
        Os/Log.c
        Os/Plugin.c
        Os/Time.c
        SoftFloat/SoftFloat.c
        SoftFloat/SoftMatrix.c
        SoftFloat/SoftVector.c)

add_library(${PROJECT_NAME} SHARED ${SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ./ThirdParty/softfloat/include)
find_library(SF_LIB softfloat)
target_link_libraries(${PROJECT_NAME} PUBLIC ${SF_LIB})
