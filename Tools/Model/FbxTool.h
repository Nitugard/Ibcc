/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_FBXTOOL_H
#define IBC_FBXTOOL_H

#include "fbxsdk.h"
#include <vector>
#include <string>
#define CPP
#define XMDL_IMPLEMENTATION
#include "Model.h"

#include "FbxTool.h"

void read_geometry_mat4(FbxNode* node, xmdl_mat* mat);
void read_normal(FbxMesh* mesh, int ctrl_point, int vertex_count, xmdl_vec3* normal);
void read_uv(FbxMesh* mesh, int ctrl_point, int tex_uv_index, int uv_layer, xmdl_vec2* uv);
void read_color(FbxMesh* mesh , int ctrl_point , int vertex_count , xmdl_vec3* color);
void read_triangle_material(FbxMesh* mesh , int tris_counter , int* triangle_material_index_array);

bool load_fbx(std::string fname, xmdl_data_handle data);

bool process_mesh(FbxNode* node, xmdl_node* e_node, xmdl_data_handle data, std::vector<std::string>* loaded_mats);
bool process_mats(FbxScene* scene, xmdl_data_handle data, std::vector<std::string>* loaded_mats);
uint32_t process_node(FbxNode* node, xmdl_data_handle data, std::vector<std::string>* loaded_mats);
int32_t load_texture(xmdl_data_handle data, FbxFileTexture* file_tex, bool sRGB);

#endif //IBC_FBXTOOL_H
