/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <GL/gl3w.h>
#include "Graphics.h"

typedef struct gfx_shader{

    i32 id;

} gfx_shader;

void gl_print_shader_err(i32 shader)
{
    GLint maxLength = 0;
    char* buffer;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = malloc(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, buffer);
    fprintf(stderr, "Shader compilation error: \n%s", buffer);
    free(buffer);
}

void gl_print_program_err(i32 shader)
{
    GLint maxLength = 0;
    char* buffer;
    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    buffer = malloc(maxLength);
    glGetProgramInfoLog(shader, maxLength, &maxLength, buffer);
    fprintf(stderr, "Program linkage error: \n%s", buffer);
    free(buffer);
}


gfx_shader_handle gfx_shader_create(const gfx_shader_desc *desc) {

    gfx_shader_handle shader = malloc(sizeof(gfx_shader));

    shader->id = glCreateProgram();

    i32 compiled = 0;
    u32 vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vs, GL_SHADER_BINARY_FORMAT_SPIR_V, desc->vs.src, desc->vs.length);
    glSpecializeShader(vs, desc->vs.entry, 0, 0, 0);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
    if (compiled)
        glAttachShader(shader->id, vs);
    else gl_print_shader_err(vs);
    compiled = 0;
    u32 fs = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &fs, GL_SHADER_BINARY_FORMAT_SPIR_V, desc->fs.src, desc->fs.length);
    glSpecializeShader(fs, desc->fs.entry, 0, 0, 0);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
    if (compiled)
        glAttachShader(shader->id, fs);
    else gl_print_shader_err(fs);

    compiled = 0;
    glLinkProgram(shader->id);
    glGetProgramiv(shader->id, GL_LINK_STATUS, &compiled);

    if (compiled) {
        fprintf(stdout, "Shader created: %s, id: %i\n", desc->name, shader->id);
    }else gl_print_program_err(shader->id);
    return shader;
}

void gfx_shader_destroy(gfx_shader_handle shader) {
    fprintf(stdout, "Shader destroyed, id: %i\n", shader->id);
    glDeleteProgram(shader->id);
    free(shader);
}



