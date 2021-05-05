#pragma once

#include "Common.h"

static char vs_source[] = STRING(
     SHADER_VERSION
     layout (location = ATTR_POSITION_LOCATION) in vec3 VERTEX_POSITION;
     layout (location = ATTR_UV_LOCATION) in vec2 VERTEX_UV;
     layout (location = ATTR_NORMAL_LOCATION) in vec3 VERTEX_NORMAL;

     uniform mat4 PROJECTION;
     uniform mat4 MODEL;
     uniform mat4 VIEW;

     out vec3 FRAGMENT_POSITION;
     out vec2 FRAGMENT_UV;
     out vec3 FRAGMENT_NORMAL;

     void main() {
         gl_Position = PROJECTION * VIEW * MODEL * vec4(VERTEX_POSITION, 1.0);
         FRAGMENT_UV = VERTEX_UV;
         FRAGMENT_NORMAL = VERTEX_NORMAL;
         FRAGMENT_POSITION = vec3(MODEL * vec4(VERTEX_POSITION.xyz, 1.0));
     }
);

static char fs_source[] = STRING(
    SHADER_VERSION
    out vec4 FragColor;

    in vec3 FRAGMENT_POSITION;
    in vec2 FRAGMENT_UV;
    in vec3 FRAGMENT_NORMAL;

    uniform mat4 PROJECTION;
    uniform mat4 MODEL;
    uniform mat4 VIEW;

    uniform mat4 LIGHT_PROJECTION;

    uniform vec3 SUN_DIRECTION;
    uniform vec3 SUN_COLOR;

    uniform vec3 AMBIENT_COLOR;
    uniform vec3 DIFFUSE_COLOR;

    uniform sampler2D TEXTURE_MAIN;
    uniform sampler2D TEXTURE_SHADOW;

    float in_shadow()
    {
        vec4 lp = LIGHT_PROJECTION * vec4(FRAGMENT_POSITION, 1);
        vec3 proj = lp.xyz / lp.w; //meaningless for ortographic projection
        proj = (proj + 1) / 2.0;
        float depth = texture(TEXTURE_SHADOW, proj.xy).r;
        float cur_depth = proj.z;
        float shadow = cur_depth > depth + 0.0002  ? 1.0 : 0.0;
        return shadow;
    }

    void main() {

        vec3 normal = normalize(FRAGMENT_NORMAL);

        vec3 lighting = vec3(1,1,1);
        vec4 color = texture(TEXTURE_MAIN, FRAGMENT_UV);

        //ambient
        vec3 ambient = AMBIENT_COLOR.xyz;

        //diffuse
        vec3 diffuse = (SUN_COLOR * clamp(dot(normalize(-SUN_DIRECTION), normal), 0, 1)) * DIFFUSE_COLOR;

        //specular

        float shadow = in_shadow();

        lighting = ambient + ((1.0 - shadow) * diffuse);
        FragColor = color * vec4(lighting, 1);
    }
);

static gfx_shader_desc lit_shader_desc = {
        .vs = {.src = vs_source},
        .fs = {.src = fs_source},
        .name = "lit_shader",
        .attrs = {
                [ATTR_POSITION_LOCATION] = {.size = 4, .num_elements = 3},
                [ATTR_UV_LOCATION] = {.size = 4, .num_elements = 2},
                [ATTR_NORMAL_LOCATION] = {.size = 4, .num_elements = 3},
        }
};
