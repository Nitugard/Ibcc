/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */



#include <cstdio>
#include "Model.h"

int main() {
    Model model = loadObj("C:\\Users\\Dragutin\\Desktop\\untitled.obj");

    FILE *f = fopen("./out.txt", "w");
    fprintf(f, "single32_t cube_vertices[] = {\n");
    for(int i=0; i<model.vertices.size(); ++i) {
        for (int j = 0; j < 3; ++j) {
            fprintf(f, "\t%f,", model.vertices[i][j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "};\n");

    fprintf(f, "single32_t cube_uvs[] = {\n");
    for(int i=0; i<model.textures.size(); ++i) {
        for (int j = 0; j < 2; ++j) {
            fprintf(f, "\t%f,", model.textures[i][j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "};\n");

    fprintf(f, "single32_t cube_indices[] = {\n");
    for(int i=0; i<model.indices.size(); ++i) {
        fprintf(f, "\t%i,", model.indices[i]);
        if ((i+1) % 3 == 0) fprintf(f, "\n");
    }
    fprintf(f, "};\n");


    fclose(f);
}