set(PROJECT_NAME Core)

set(SRC
        Containers/Array.c
        Containers/String.c
        Containers/Hash.c
        Os/Allocator.c
        Os/File.c
        Os/Log.c
        Os/Time.c
        Math/Math.c)

add_library(${PROJECT_NAME} ${SRC})

