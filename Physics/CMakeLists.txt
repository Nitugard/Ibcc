
target_include_directories(${PROJECT_NAME} PRIVATE ./ThirdParty/softfloat/include)
find_library(SF_LIB ../Physics/SoftFloat HINTS ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SF_LIB})