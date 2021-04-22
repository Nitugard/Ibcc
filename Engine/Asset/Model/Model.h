/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef IBC_MODEL_H
#define IBC_MODEL_H

#include <stdint.h>
#include <stdbool.h>


#ifdef CPP
#include <vector>
#include <string>
#include <fstream>
#define VECTOR(type) std::vector<type>
#define STRING std::string
#else
#define VECTOR(type) type*
#define STRING char const*
#endif

typedef struct xmdl_mat {
    float data[16];
} xmdl_mat;

typedef struct xmdl_vec4{
    float x, y, z, w;
} xmdl_vec4;

typedef struct xmdl_vec3{
    float x, y, z;
} xmdl_vec3;

typedef struct xmdl_vec2{
    float x, y;
} xmdl_vec2;

typedef enum xmdl_type{
    XMDL_UNKNOWN,
    XMDL_MESH,
    XMDL_LIGHT,
    XMDL_CAMERA,
    XMDL_BONE,
    XMDL_NODE
} xmdl_type;

typedef struct xmdl_vertex {
    xmdl_vec3 pos;
    xmdl_vec3 color;
    xmdl_vec2 uv;
    xmdl_vec3 normal;
} xmdl_vertex;

typedef struct xmdl_texture{
    VECTOR(char) buffer;
    int32_t size;
} xmdl_texture;

typedef struct xmdl_material {

    xmdl_vec4 diffuse_color;
    xmdl_vec4 specular_color;

    int32_t diffuse_tex_id;
    int32_t specular_tex_id;
} xmdl_material;

typedef struct xmdl_light{
    xmdl_vec4 color;
    float intensity;

} xmdl_light;

typedef struct xmdl_camera{

} xmdl_camera;

typedef struct xmdl_mesh
{
    VECTOR(xmdl_vertex) vertices;
    int32_t vertices_count;

    int32_t material_id;

} xmdl_mesh;

typedef struct xmdl_node{
    xmdl_mat tr_world;
    xmdl_mat tr_parent;

    xmdl_type block_type;
    int32_t block_index;

    int32_t child_count;
    VECTOR(int32_t) children_id;
} xmdl_node;

typedef struct xmdl_data{

    int32_t version;

    int32_t mesh_count;
    VECTOR(xmdl_mesh) meshes;

    int32_t material_count;
    VECTOR(xmdl_material) materials;

    int32_t texture_count;
    VECTOR(xmdl_texture) textures;

    int32_t node_count;
    VECTOR(xmdl_node) nodes;

} xmdl_data;

xmdl_mat xmdl_mat_identity();

typedef xmdl_data* xmdl_data_handle;
#ifdef CPP
#ifdef XMDL_IMPLEMENTATION
void write(std::ofstream& stream, void* buffer, uint32_t size)
{
    stream.write((const char*) buffer, size);
}

void write_int32(std::ofstream& stream, int32_t val){
    write(stream, &val, sizeof(int32_t));
}

void write_binary(std::string path, xmdl_data_handle data) {
    size_t last_index = path.find_last_of(".");
    std::string raw_name = path.substr(0, last_index);
    raw_name += ".xmdl";

    std::ofstream file(raw_name, std::ios::binary);
    write_int32(file, data->version);
    write_int32(file, data->meshes.size());
    write_int32(file, data->materials.size());
    write_int32(file, data->textures.size());
    write_int32(file, data->nodes.size());

    //Writing meshes
    for (auto mesh_i : data->meshes) {
        write_int32(file, mesh_i.vertices.size());
        write_int32(file, mesh_i.material_id);
        write(file, mesh_i.vertices.data(), sizeof(xmdl_vertex) * mesh_i.vertices.size());
    }

    //Writing nodes
    for (auto node_i : data->nodes) {
        write(file, &(node_i.tr_world), sizeof(xmdl_mat));
        write(file, &(node_i.tr_parent), sizeof(xmdl_mat));
        write(file, &(node_i.block_type), sizeof(xmdl_type));
        write(file, &(node_i.block_index), sizeof(int32_t));
        write_int32(file, node_i.children_id.size());
        write(file, node_i.children_id.data(), sizeof(int32_t) * node_i.child_count);
    }

    file.close();
}
#endif
#else
#ifdef XMDL_IMPLEMENTATION

#endif
#endif

#endif //IBC_MODEL_H
