#pragma once

#include "Common.h"

static const char vs_source[] = STRING(
    SHADER_VERSION
    layout (location = ATTR_POSITION_LOCATION) in vec2 v_pos;
    layout (location = ATTR_UV_LOCATION) in vec2 v_uv;

    uniform mat4 model;

    out vec2 _uv;
    void main() {
        gl_Position = model * vec4(v_pos.x, v_pos.y, 0, 1.0);
        _uv = v_uv;
    }
);

static const char fs_source[] = STRING(
    SHADER_VERSION
    out vec4 FragColor;

    uniform sampler2D font_tex;
    in vec2 _uv;


    void main() {
        vec4 tex = texture(font_tex, _uv);
        FragColor = tex;
    }
);

static gfx_shader_desc font_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "font_shader",
        .attrs = {
                [ATTR_POSITION_LOCATION] = {.size = 4, .num_elements = 2},
                [ATTR_UV_LOCATION] = {.size = 4, .num_elements = 2},
        }
};
