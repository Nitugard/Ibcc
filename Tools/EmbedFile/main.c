/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/include/stbi/stbi.h>

#include <Os/File.h>
#include <stdio.h>

int main(int argc, char** argv) {
    uint32_t len;
    const char *fname = argv[1];
    unsigned char *buffer = file_mmap(fname, &len);
    file_hndl hndl = file_open(argv[2], "w");
    FILE* f = file_get_hndl(hndl);
    int32_t width, height, channels;
    unsigned char* img_buffer = stbi_load_from_memory(buffer, len, &width, &height, &channels, 0);
    if(img_buffer != 0) {
        fprintf(f, "const uint32 file_width = %i;\n", width);
        fprintf(f, "const uint32 file_height = %i;\n", height);
        fprintf(f, "const uint32 file_channels = %i;\n", channels);
        len = width * height * channels;
        buffer = img_buffer;
    }

    fprintf(f, "const unsigned char file[%u] = {\n\t", len);
    for (int32_t i = 0; i < len; ++i) {
        fprintf(f, "0x%02x", *(buffer + i));
        if(i + 1 != len) fprintf(f, ",");
        if((i + 1 ) % 32 == 0)fprintf(f, "\n\t");

    }
    fprintf(f, "\n};\n");

    return 0;
}