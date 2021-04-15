#pragma once

#include "Common.h"

#define position_attr 0
#define uv_attr 1

static const char vs_source[] = CONCATENATE(
        SH_VER,
     layout (location = position_attr) in vec3 v_pos;
     layout (location = uv_attr) in vec2 v_uv;

     layout(row_major) uniform matrices{
         mat4 projection;
         mat4 modelview;
     };

     out vec2 _uv;

     void main() {
         gl_Position = projection * vec4(v_pos.xyz, 1.0);
         _uv = v_uv;
     }
);

static const char fs_source[] = CONCATENATE(
        SH_VER,
    out vec4 FragColor;

    in vec2 _uv;

    uniform sampler2D ourTexture;

    void main()
    {
        FragColor = texture(ourTexture, _uv);
    }
);

static gfx_shader_desc unlit_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "unlit_shader",
        .attrs = {
                [position_attr] = {.size = 4, .num_elements = 3},
                [uv_attr] = {.size = 4, .num_elements = 2},
        }
};
