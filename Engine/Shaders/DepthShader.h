#pragma once

#include "Common.h"


static const char vs_source[] = STRING(
        SHADER_VERSION
        layout (location = ATTR_POSITION_LOCATION) in vec2 ATTR_POSITION_NAME;
        layout (location = ATTR_UV_LOCATION) in vec2 ATTR_UV_NAME;
        layout (location = ATTR_COLOR_LOCATION) in vec4 ATTR_COLOR_NAME;

        uniform mat4 projection;

        out vec2 fATTR_UV_NAME;
        out vec4 fATTR_COLOR_NAME;
        void main() {
            gl_Position = projection * vec4(ATTR_POSITION_NAME, 0, 1.0);
            fATTR_UV_NAME = ATTR_UV_NAME;
            fATTR_COLOR_NAME = ATTR_COLOR_NAME;
        }
);

static const char fs_source[] = STRING(
        SHADER_VERSION
        out vec4 FragColor;

        uniform sampler2D COLOR_TEXTURE_NAME;
        in vec2 fATTR_UV_NAME;
        in vec4 fATTR_COLOR_NAME;

        float near = 0.1;
        float far  = 100.0;

        float LinearizeDepth(float depth)
        {
            float z = depth * 2.0 - 1.0;
            return (2.0 * near * far) / (far + near - z * (far - near));
        }

        void main() {
            vec4 tex = texture(COLOR_TEXTURE_NAME, fATTR_UV_NAME);
            float depth = LinearizeDepth(tex.r) / far;
            FragColor = vec4(vec3(depth), 1.0);
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
