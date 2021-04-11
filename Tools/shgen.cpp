/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <string>
#include <iostream>
#include <filesystem>
#include <vector>

int main()
{
    std::string lang = "glsl";
    std::string path = "./Assets/Shaders/";
    for (const auto & entry : std::filesystem::directory_iterator(path))
    {
        if(entry.is_directory()) continue;

        system(("glslcc.exe -S -l " + lang +
        " -r -i " + entry.path().string() +
        " -o " + path + lang+ "/"+entry.path().filename().replace_extension(lang).string() )
        .c_str());
    }

    return 0;
}