#pragma once

#include "Common.h"

#define position_attr 0
#define uv_attr 1
#define normal_attr 2

static const char vs_source[] = CONCATENATE(
        SH_VER,
     layout (location = position_attr) in vec4 v_pos;
     layout (location = uv_attr) in vec2 v_uv;
     layout (location = normal_attr) in vec3 v_normal;

     layout(row_major) uniform matrices{
         mat4 model;
         mat4 view;
         mat4 projection;

         vec3 light_color;
         vec3 light_pos;
         vec3 cam_pos;
     };
     out vec3 _pos;
     out vec2 _uv;
     out vec3 _normal;
     void main() {
         gl_Position = projection * view * model * vec4(v_pos.xyz, 1.0);
         _uv = v_uv;
         _normal = v_normal;
         _pos = vec3(model * vec4(v_pos.xyz, 1.0));
     }
);

static const char fs_source[] = CONCATENATE(
        SH_VER,
    out vec4 FragColor;

    in vec3 _pos;
    in vec2 _uv;
    in vec3 _normal;

    uniform sampler2D ourTexture;

    layout(row_major) uniform matrices{
        mat4 model;
        mat4 view;
        mat4 projection;

        vec3 light_color;
        vec3 light_pos;
        vec3 cam_pos;
    };

    void main()
    {
        vec3 norm = normalize(_normal);
        vec3 light_dir = normalize(light_pos - _pos);
        float diff = max(dot(norm, light_dir), 0.0);
        vec3 view_dir = normalize(cam_pos - _pos);
        vec3 reflect_dir = reflect(-light_dir, norm);
        float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 64);
        vec3 specular = vec3(0.8f * spec);
        vec3 diffuse = vec3(diff);
        vec3 ambient = vec3(0.8, 0.5, 0.5);
        FragColor = vec4((diffuse + specular + ambient) * _normal,1);
    }
);

static gfx_shader_desc unlit_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "unlit_shader",
        .attrs = {
                [position_attr] = {.size = 4, .num_elements = 3},
                [uv_attr] = {.size = 4, .num_elements = 2},
                [normal_attr] = {.size = 4, .num_elements = 3},

        }
};
