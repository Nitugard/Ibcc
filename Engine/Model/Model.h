/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include <stdbool.h>

#ifndef API
#define API
#endif

typedef enum mdl_primitive_type{
    MDL_PRIMITIVE_TYPE_POINTS,
    MDL_PRIMITIVE_TYPE_LINES,
    MDL_PRIMITIVE_TYPE_LINE_LOOP,
    MDL_PRIMITIVE_TYPE_LINE_STRIP,
    MDL_PRIMITIVE_TYPE_TRIANGLES,
    MDL_PRIMITIVE_TYPE_TRIANGLE_STRIP,
    MDL_PRIMITIVE_TYPE_TRIANGLE_FAN,
} mdl_primitve_type;

typedef enum mdl_node_type{
    MDL_NODE_NODE,
    MDL_NODE_MESH,
    MDL_NODE_LIGHT,
    MDL_NODE_CAMERA,
    MDL_NODE_BONE,
} mdl_node_type;

typedef enum mdl_vertex_attribute_type{
    MDL_VERTEX_ATTRIBUTE_INVALID = 0,
    MDL_VERTEX_ATTRIBUTE_POSITION = 0x1,
    MDL_VERTEX_ATTRIBUTE_UV = 0x2,
    MDL_VERTEX_ATTRIBUTE_NORMAL = 0x4,
    MDL_VERTEX_ATTRIBUTE_COLOR = 0x8,
    MDL_VERTEX_ATTRIBUTE_TANGENT = 0x10,
    MDL_VERTEX_ATTRIBUTE_WEIGHTS = 0x20,
    MDL_VERTEX_ATTRIBUTE_JOINTS = 0x40,
} mdl_vertex_attribute_type;

typedef struct mdl_texture{
    char *name;
    void * buffer;
    int32_t size;

    int32_t width;
    int32_t height;
    int32_t channels;
    bool valid;
} mdl_texture;

typedef struct mdl_material {
    char* name;
    bool unlit;
    bool valid;
    float alpha_cutoff;
    int32_t color_texture_id;
    float color_factor[4];
} mdl_material;

typedef struct mdl_light{
    char* name;
} mdl_light;

typedef struct mdl_camera{
    char* name;
} mdl_camera;

typedef struct mdl_attribute{
    mdl_vertex_attribute_type type;
    int32_t offset;
    int32_t count;
    int32_t element_size;
} mdl_attribute;

typedef struct mdl_primitive {

    enum mdl_primitive_type primitive_type;

    int32_t attributes_flag;
    int32_t attributes_count;
    mdl_attribute* attributes;

    int32_t vertices_count;
    void *vertices;

    int32_t indices_count;
    uint32_t *indices;

    uint32_t vertex_stride;

    uint32_t material_id;
}mdl_primitive;

typedef struct mdl_mesh
{
    char* name;

    uint32_t primitives_count;
    mdl_primitive* primitives;

} mdl_mesh;

typedef struct mdl_node{

    char * name;

    float tr_world[16];
    float tr_local[16];

    int32_t mesh_index;
    int32_t camera_index;
    int32_t light_index;

    int32_t children_count;
    int32_t * children_id;

    int32_t parent_id;
} mdl_node;

typedef struct mdl_data{

    char * name;

    int32_t meshes_count;
    mdl_mesh * meshes;

    int32_t materials_count;
    mdl_material * materials;

    int32_t textures_count;
    mdl_texture * textures;

    int32_t nodes_count;
    mdl_node * nodes;


} mdl_data;

typedef struct mdl_desc{
    const char* path;
    bool load_textures;
} mdl_desc;

typedef struct mdl_data* mdl_handle;

API mdl_handle mdl_load(mdl_desc* desc);
API void mdl_unload(mdl_handle handle);

#endif //MODEL_H
