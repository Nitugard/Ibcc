#pragma once

#include "Common.h"

#define position_attr 0
#define color_attr 1


static const i8 vs_source[] = CONCATENATE(
        SH_VER,
     layout (location = position_attr) in vec3 v_pos_in;
     layout (location = color_attr) in vec4 v_color_in;

     layout(row_major) uniform matrices{
         mat4 projection;
         mat4 modelview;
     };

     out vec4 v_color_out;
     void main() {
         gl_Position = projection * vec4(v_pos_in.xyz, 1.0);
         v_color_out = vec4(v_color_in.xyz, 1.0);
     }
);

static const i8 fs_source[] = CONCATENATE(
        SH_VER,
    out vec4 FragColor;
    in vec4 v_color_out;

    void main()
    {
        FragColor = v_color_out;
    }
);

static gfx_shader_desc unlit_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "unlit_shader",
        .attrs = {
                [color_attr] = {.size = 4, .num_elements = 4},
                [position_attr] = {.size = 4, .num_elements = 3},
        }
};
