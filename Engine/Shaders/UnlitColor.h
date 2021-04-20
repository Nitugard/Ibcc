#pragma once

#include "Common.h"

#define position_attr 0
#define color_attr 1

static const char vs_source[] = CONCATENATE(
    SH_VER,
    layout (location = position_attr) in vec4 v_pos;
    layout (location = color_attr) in vec3 v_color;

    layout(row_major) uniform matrices{
        mat4 model;
        mat4 view;
        mat4 projection;

        vec3 light_color;
        vec3 light_pos;
        vec3 cam_pos;
    };

    out vec3 _color;
    void main() {
        gl_Position = projection * view * model * vec4(v_pos.xyz, 1.0);
        _color = v_color;
    }
);

static const char fs_source[] = CONCATENATE(
    SH_VER,
    out vec4 FragColor;

    in vec3 _color;

    uniform sampler2D ourTexture;

    void main()
    {
        FragColor = vec4(_color,1);
    }
);

static gfx_shader_desc unlit_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "unlit_color_shader",
        .attrs = {
                [position_attr] = {.size = 4, .num_elements = 3},
                [color_attr] = {.size = 4, .num_elements = 3},
        }
};
