set(PROJECT_NAME Core)
set(SRC ./Containers/Array.c
        ./Plugin/Plugin.c
        ./FixedPoint/FixedPoint.c
        ./FixedPoint/FixedVector.c
        ./FixedPoint/FixedMatrix.c
        ./SoftFloat/SoftFloat.c
        ./SoftFloat/SoftVector.c
        ./SoftFloat/SoftMatrix.c
        ./Os/Allocator.c
        ./Log/Log.c
        ./Os/File.c
        ./Os/Time.c
        ./HandmadeMath/HandmadeMath.c
        )

add_library(${PROJECT_NAME} SHARED ${SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ./ThirdParty/softfloat/include)
find_library(SF_LIB softfloat HINTS ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SF_LIB})
