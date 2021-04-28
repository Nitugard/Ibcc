/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_COMMON_H
#define IBC_COMMON_H

#define ATTR_POSITION_LOCATION 0
#define ATTR_COLOR_LOCATION 1
#define ATTR_UV_LOCATION 2
#define ATTR_NORMAL_LOCATION 3
#define ATTR_TANGENT_LOCATION 4
#define ATTR_WEIGHTS_LOCATION 5
#define ATTR_JOINTS_LOCATION 6

#define COLOR_TEXTURE_NAME color_texture
#define ATTR_POSITION_NAME v_pos
#define ATTR_UV_NAME v_uv
#define ATTR_COLOR_NAME v_color
#define ATTR_NORMAL_NAME v_normal

#define SHADER_VERSION #version 330\n

#define SHADER_MVP \
    uniform mat4 model;\
    uniform mat4 view;\
    uniform mat4 projection;\


#define __STRING(x) #x
#define STRING(x) __STRING(x)

#endif //IBC_COMMON_H
