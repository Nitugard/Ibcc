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

#define TEXTURE_MAIN color_texture
#define TEXTURE_SHADOW shadow_texture
#define PROJECTION projection
#define MODEL model
#define VIEW view
#define LIGHT_VIEW light_view
#define LIGHT_PROJECTION light_projection

#define VERTEX_POSITION v_pos
#define VERTEX_UV v_uv
#define VERTEX_COLOR v_color
#define VERTEX_NORMAL v_normal

#define FRAGMENT_POSITION f_pos
#define FRAGMENT_UV f_uv
#define FRAGMENT_COLOR f_color
#define FRAGMENT_NORMAL f_normal

#define SUN_DIRECTION sun_direction
#define SUN_COLOR sun_color
#define ENABLE_SHADOWS enable_shadows

#define AMBIENT_COLOR ambient_color
#define DIFFUSE_COLOR diffuse_color

#define SHADER_VERSION #version 330\n

#define __STRING(x) #x
#define STRING(x) __STRING(x)

#endif //IBC_COMMON_H
