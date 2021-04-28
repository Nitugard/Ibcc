#pragma once

#include "Common.h"

static const char vs_source[] = STRING(
    SHADER_VERSION
    layout (location = ATTR_POSITION_LOCATION) in vec3 v_pos;
    layout (location = ATTR_COLOR_LOCATION) in vec3 v_color;

        SHADER_MVP

    out vec3 _color;
    void main() {
        gl_Position = projection * view * vec4(v_pos, 1.0);
        _color = v_color;
    }
);

static const char fs_source[] = STRING(
    SHADER_VERSION
    out vec4 FragColor;

    in vec3 _color;

    void main()
    {
        FragColor = vec4(_color,1);
    }
);

static gfx_shader_desc unlit_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "unlit_vertex_color_shader",
        .attrs = {
                [ATTR_POSITION_LOCATION] = {.size = 4, .num_elements = 3},
                [ATTR_COLOR_LOCATION] = {.size = 4, .num_elements = 3},
        }
};
