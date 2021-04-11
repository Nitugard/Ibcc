set(PROJECT_NAME Core)
set(SRC ./Containers/Array.c
        ./Plugin/Plugin.c
        ./FixedMath/FixedMath.c
        ./Os/Allocator.c
        ./Log/Log.c
        ./Os/File.c
        ./Os/Time.c
        )
add_library(${PROJECT_NAME} SHARED ${SRC})