#pragma once

#include "Common.h"


static const char vs_source[] = STRING(
    SHADER_VERSION
    layout (location = ATTR_POSITION_LOCATION) in vec2 VERTEX_POSITION;
    layout (location = ATTR_UV_LOCATION) in vec2 VERTEX_UV;

    uniform mat4 PROJECTION;
    out vec2 FRAGMENT_UV;
    void main() {
        gl_Position = PROJECTION * vec4(VERTEX_POSITION, 0, 1.0);
        FRAGMENT_UV = VERTEX_UV;
    }
);

static const char fs_source[] = STRING(
    SHADER_VERSION
    out vec4 FragColor;

    uniform sampler2D TEXTURE_MAIN_COLOR;
    in vec2 FRAGMENT_UV;

    void main() {
        vec4 tex = texture(TEXTURE_MAIN_COLOR, FRAGMENT_UV);
        FragColor = tex;
    }
);

static gfx_shader_desc sprite_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "sprite_shader",
        .attrs = {
                [ATTR_POSITION_LOCATION] = {
                        .size = 4,
                        .num_elements = 2,
                },
                [ATTR_UV_LOCATION] = {
                        .size = 4,
                        .num_elements = 2,
                },
        }
};
