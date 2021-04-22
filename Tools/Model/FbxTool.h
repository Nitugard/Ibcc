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
// https://raw.githubusercontent.com/HackerEva/FBXSDK_LOADER_and_Render/

#include "FbxTool.h"

void read_normal(FbxMesh* pMesh, int ctrlPointIndex, int vertexCounter, xmdl_vec3* pNormal);

bool process_mesh(FbxNode* node, xmdl_node* e_node, xmdl_data_handle data);

uint32_t process_node(FbxNode* node, xmdl_data_handle data);

bool load_fbx(std::string fname, xmdl_data_handle data);


#endif //IBC_FBXTOOL_H
