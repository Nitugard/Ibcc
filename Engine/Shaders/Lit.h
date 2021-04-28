#pragma once

#include "Common.h"

static char vs_source[] = STRING(
     SHADER_VERSION
     layout (location = ATTR_POSITION_LOCATION) in vec4 vPos;
     layout (location = ATTR_UV_LOCATION) in vec2 vUv;
     layout (location = ATTR_NORMAL_LOCATION) in vec3 vNormal;

        SHADER_MVP


     out vec3 fPos;
     out vec2 fUv;
     out vec3 fNormal;
     void main() {
         gl_Position = projection * view * model * vec4(vPos.xyz, 1.0);
         fUv = vUv;
         fNormal = vNormal;
         fPos = vec3(model * vec4(vPos.xyz, 1.0));
     }
);

static char fs_source[] = STRING(
    SHADER_VERSION
    out vec4 FragColor;

    in vec3 fPos;
    in vec2 fUv;
    in vec3 fNormal;

        SHADER_MVP

        uniform vec4 colorFactor;
        uniform sampler2D colorTexture;

        void main() {
            vec4 color = texture(colorTexture, fUv);
            if(color.a < 0.5)
            {
                discard;
            }
            FragColor = color * colorFactor;
        }
);

static gfx_shader_desc lit_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "lit_shader_desc",
        .attrs = {
                [ATTR_POSITION_LOCATION] = {.size = 4, .num_elements = 3, .name = ""},
                [ATTR_UV_LOCATION] = {.size = 4, .num_elements = 2, .name = ""},
                [ATTR_NORMAL_LOCATION] = {.size = 4, .num_elements = 3, .name = ""},
        }
};
