#pragma once

#include "Common.h"


static const char vs_source[] = STRING(
        SHADER_VERSION
        layout (location = ATTR_POSITION_LOCATION) in vec2 VERTEX_POSITION;
        layout (location = ATTR_UV_LOCATION) in vec2 VERTEX_UV;
        layout (location = ATTR_COLOR_LOCATION) in vec4 VERTEX_COLOR;

        uniform mat4 PROJECTION;
        out vec2 FRAGMENT_UV;
        out vec4 FRAGMENT_COLOR;

        void main() {
            gl_Position = PROJECTION * vec4(VERTEX_POSITION, 0, 1.0);
            FRAGMENT_UV = VERTEX_UV;
            FRAGMENT_COLOR = VERTEX_COLOR;
        }
);

static const char fs_source[] = STRING(
        SHADER_VERSION
        out vec4 FragColor;

        uniform sampler2D TEXTURE_MAIN;
        in vec2 FRAGMENT_UV;
        in vec4 FRAGMENT_COLOR;

        void main() {
            vec4 tex = texture(TEXTURE_MAIN, FRAGMENT_UV);
            FragColor = vec4(vec3(tex.r), 1);
        }
);

static gfx_shader_desc sprite_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "font_shader",
        .attrs = {
                [ATTR_POSITION_LOCATION] = {
                        .size = 4,
                        .num_elements = 2,
                },
                [ATTR_UV_LOCATION] = {
                        .size = 4,
                        .num_elements = 2,
                },
                [ATTR_COLOR_LOCATION] = {
                        .size = 4,
                        .num_elements = 4,
                },
        }
};
