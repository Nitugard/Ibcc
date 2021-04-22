project(Tools CXX)

set(FBX_TOOL_NAME FbxTool)

add_executable(${FBX_TOOL_NAME} ./Model/FbxTool.cpp)
target_link_directories(${FBX_TOOL_NAME} PRIVATE $ENV{FBX_SDK_PATH}/lib/vs2017/x64/debug)
target_include_directories(${FBX_TOOL_NAME} PRIVATE
        $ENV{FBX_SDK_PATH}/include
        ../Engine/Asset/Model/
        ./Thirdparty/Argparse/
        ./Thirdparty/stb_image/
        )
target_link_libraries(${FBX_TOOL_NAME} PRIVATE libfbxsdk)
target_compile_features(${FBX_TOOL_NAME} PRIVATE cxx_std_17)

