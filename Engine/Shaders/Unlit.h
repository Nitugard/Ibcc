#pragma once

#include <Graphics/Graphics.h>
#include "Common.h"

static const char vs_source[] = STRING(
    SHADER_VERSION
    layout (location = ATTR_POSITION_LOCATION) in vec3 VERTEX_POSITION;
    layout (location = ATTR_COLOR_LOCATION) in vec3 VERTEX_COLOR;

    uniform mat4 PROJECTION;
    uniform mat4 VIEW;

    out vec3 FRAGMENT_POSITION;
    out vec2 FRAGMENT_UV;
    out vec3 FRAGMENT_NORMAL;

    out vec3 FRAGMENT_COLOR;
    void main() {
        gl_Position = PROJECTION * VIEW * vec4(v_pos, 1.0);
        FRAGMENT_COLOR = VERTEX_COLOR;
    }
);

static const char fs_source[] = STRING(
    SHADER_VERSION
    out vec4 FragColor;

    in vec3 FRAGMENT_COLOR;

    void main()
    {
        FragColor = vec4(FRAGMENT_COLOR, 1);
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
