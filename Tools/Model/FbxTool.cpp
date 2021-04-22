/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <cassert>
#define FBXSDK_SHARED
#include "FbxTool.h"
#include <fstream>
#include <map>
#include <string>
#include "Argparse.h"
#include <algorithm>
#include <numeric>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/*
 * Code used from:
 * -https://www.gamedev.net/tutorials/programming/graphics/how-to-work-with-fbx-sdk-r3582/
 * -https://github.com/HackerEva/FBXSDK_LOADER_and_Render/blob/master/FBXLoader/FBXLoader.cpp
 * -https://programmersought.com/article/96315157947/
 * -http://docs.autodesk.com/FBX/2014/ENU/FBX-SDK-Documentation/index.html?url=files/GUID-3E0DCCD0-5F9A-44D0-8D5E-423237D49DB6.htm,topicNumber=d30e10256
 */

int main(int argc, const char** argv) {


    ArgumentParser parser;
    parser.addArgument("-f", "--filename", 1, false);
    parser.parse(argc, argv);
    std::string input = parser.retrieve<std::string>("filename");
    xmdl_data data;
    data.version = (1) | (0 << 8) | (0 << 16);
    data.nodes = {};
    data.meshes = {};
    data.textures = {};
    data.materials = {};
    load_fbx(std::string(input.c_str()), &data);
    write_binary(std::string(input.c_str()), &data);
    return 0;
}

template <typename T, typename Compare>
std::vector<std::size_t> sort_permutation(
        const std::vector<T>& vec,
        Compare  compare)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
              [&](std::size_t i, std::size_t j){ return compare(vec[i], vec[j]); });
    return p;
}

template <typename T>
void apply_permutation_in_place_multiple(
        std::vector<T>& vec,
        const std::vector<std::size_t>& p, int32_t multiple)
{
    std::vector<bool> done(p.size());
    for (std::size_t i = 0; i < p.size(); ++i)
    {
        if (done[i])
            continue;
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j)
        {
            for(int k=0; k<multiple; ++k) {
                std::swap(vec[prev_j * multiple + k], vec[j * multiple + k]);
            }

            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}


bool load_fbx(std::string fname, xmdl_data_handle data) {
    FbxManager *manager = nullptr;
    manager = FbxManager::Create();
    FbxIOSettings *io_settings = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(io_settings);

    FbxImporter *importer = FbxImporter::Create(manager, "");
    FbxScene *fbx_scene = FbxScene::Create(manager, "scene");

    bool success = importer->Initialize(fname.c_str(), -1, manager->GetIOSettings());
    if (!success) {
        fprintf(stderr, "Could not initialize importer: %s\n", fname.c_str());
        FbxStatus& status = importer->GetStatus();
        fprintf(stderr, "Error: %s\n", status.GetErrorString());
        return false;
    }

    importer->Import(fbx_scene);
    importer->Destroy();

    FbxNode *root = fbx_scene->GetRootNode();
    if (root == nullptr) {
        fprintf(stderr, "Could not find root node\n");
        return false;
    }

    std::vector<std::string> loaded_mats;
    process_mats(fbx_scene, data, &loaded_mats);
    process_node(root, data, &loaded_mats);
    return true;
}

FbxFileTexture* get_tex_property(FbxProperty* pProp) {
    return (FbxFileTexture*)pProp->GetSrcObject(FbxCriteria::ObjectType(FbxFileTexture::ClassId));
}


void d3_to_f3(void* f, double const* d)
{
    for(int i=0;i<3; ++i) *((float*)f + i) = (float)*(d + i);
}

bool process_mats(FbxScene* scene, xmdl_data_handle data,std::vector<std::string>* loaded_mats)
{
    int32_t mats_count = scene->GetMaterialCount();
    data->materials.resize(mats_count);
    fprintf(stdout, "Loaded material count %i\n", mats_count);
    for(uint32_t i=0; i<mats_count; ++i) {
        FbxSurfaceMaterial *mat = scene->GetMaterial(i);
        xmdl_material* xmat = data->materials.data() + i;
        //xmat->name = mat->GetName();
        loaded_mats->push_back(mat->GetName());
        fprintf(stdout, "Loaded material %s\n", mat->GetName());
        memset(xmat, 0, sizeof(xmdl_material));
        if (mat->GetClassId() == FbxSurfacePhong::ClassId) {
            //phong
            auto *pmat = (FbxSurfacePhong *) mat;
            d3_to_f3(&(xmat->diffuse_color), pmat->Diffuse.Get().mData);
            d3_to_f3(&(xmat->specular_color), pmat->Specular.Get().mData);
            d3_to_f3(&(xmat->ambient_color), pmat->Ambient.Get().mData);

            xmat->diffuse_factor = pmat->DiffuseFactor.Get();
            xmat->ambient_factor = pmat->AmbientFactor.Get();
            xmat->specular_factor= pmat->SpecularFactor.Get();
            xmat->opacity = pmat->TransparencyFactor.Get();

            xmat->diffuse_tex_id = load_texture(data, get_tex_property(&pmat->Diffuse), true);
            xmat->specular_tex_id = load_texture(data, get_tex_property(&pmat->Specular), true);
        }
        if (mat->GetClassId() == FbxSurfaceLambert::ClassId) {
            //phong
            auto *pmat = (FbxSurfaceLambert *) mat;
            d3_to_f3(&(xmat->diffuse_color), pmat->Diffuse.Get().mData);
            d3_to_f3(&(xmat->ambient_color), pmat->Ambient.Get().mData);

            xmat->diffuse_factor = pmat->DiffuseFactor.Get();
            xmat->ambient_factor = pmat->AmbientFactor.Get();
            xmat->opacity = pmat->TransparencyFactor.Get();

            xmat->diffuse_tex_id = load_texture(data, get_tex_property(&pmat->Diffuse), true);
            xmat->specular_tex_id = -1;
        }
    }

    return true;
}

int32_t load_texture(xmdl_data_handle data, FbxFileTexture* file_tex, bool sRGB) {
    if (!file_tex) {
        fprintf(stderr, "File texture invalid handle\n");
        return -1;
    }

    const char *path = file_tex->GetFileName();
    auto it = std::find_if(data->textures.begin(), data->textures.end(),
                           [path](const xmdl_texture &a) { return strcmp(a.name.c_str(), path) == 0; });
    if (it != data->textures.end()) {
        return std::distance(data->textures.begin(), it);
    }

    int channelsOut = 4;
    int width, height, channelsIn;
    uint8_t *img_data = stbi_load(path, &width, &height, &channelsIn, channelsOut);
    if (img_data == nullptr) {
        fprintf(stderr, "Stb_image couldn't load %s\n", path);
        return -1;
    }

    size_t size = width * height * channelsOut;

    xmdl_texture tex;
    tex.buffer = {};
    tex.name = path;
    tex.buffer.resize(size);
    memcpy((void *) tex.buffer.data(), img_data, size);
    STBI_FREE(img_data);
    tex.sRGB = sRGB;
    tex.width = width;
    tex.height = height;
    tex.stride = width;
    tex.size = size;
    fprintf(stdout, "Loaded texture %s\n", file_tex);
    data->textures.push_back(tex);
    return data->textures.size() - 1;
}


uint32_t process_node(FbxNode* node, xmdl_data_handle data, std::vector<std::string>* loaded_mats) {
    FbxNodeAttribute *attribute = node->GetNodeAttribute();
    xmdl_node e_node = {};
    e_node.children_id = {};
    e_node.block_index = -1;
    e_node.block_type = XMDL_NODE;
    e_node.child_count = node->GetChildCount();
    e_node.name = node->GetName();

    read_geometry_mat4(node, &(e_node.tr_parent));

    fprintf(stdout, "Loaded node %s\n", node->GetName());


    if (attribute) {
        switch (attribute->GetAttributeType()) {
            case FbxNodeAttribute::eUnknown:
                break;
            case FbxNodeAttribute::eNull:
                break;
            case FbxNodeAttribute::eMarker:
                break;
            case FbxNodeAttribute::eSkeleton:
                break;
            case FbxNodeAttribute::eMesh:
                process_mesh(node, &e_node, data, loaded_mats);
                break;
            case FbxNodeAttribute::eNurbs:
                break;
            case FbxNodeAttribute::ePatch:
                break;
            case FbxNodeAttribute::eCamera:
                break;
            case FbxNodeAttribute::eCameraStereo:
                break;
            case FbxNodeAttribute::eCameraSwitcher:
                break;
            case FbxNodeAttribute::eLight:
                break;
            case FbxNodeAttribute::eOpticalReference:
                break;
            case FbxNodeAttribute::eOpticalMarker:
                break;
            case FbxNodeAttribute::eNurbsCurve:
                break;
            case FbxNodeAttribute::eTrimNurbsSurface:
                break;
            case FbxNodeAttribute::eBoundary:
                break;
            case FbxNodeAttribute::eNurbsSurface:
                break;
            case FbxNodeAttribute::eShape:
                break;
            case FbxNodeAttribute::eLODGroup:
                break;
            case FbxNodeAttribute::eSubDiv:
                break;
            case FbxNodeAttribute::eCachedEffect:
                break;
            case FbxNodeAttribute::eLine:
                break;
        }
    }

    uint32_t node_id = data->nodes.size();
    data->nodes.push_back(e_node);

    for (uint32_t i = 0; i < e_node.child_count; ++i) {
        uint32_t child_id = process_node(node->GetChild(i), data, loaded_mats);
        if(child_id == -1) continue;
        data->nodes[node_id].children_id.push_back(child_id);
    }

    return node_id;
}

bool process_mesh(FbxNode *node, xmdl_node* e_node, xmdl_data_handle data, std::vector<std::string>* loaded_mats) {

    FbxMesh *mesh = node->GetMesh();
    if (mesh == nullptr)
        return false;

    fprintf(stdout, "Loaded mesh %s\n", mesh->GetName());

    e_node->block_type = XMDL_MESH;
    xmdl_mesh e_mesh;
    e_mesh.draw_calls = {};
    e_mesh.vertices_count = 0;
    e_mesh.vertices = {};
    int32_t triangle_count = mesh->GetPolygonCount();

    //control points is synonym for vertex
    FbxVector4 *ctrl_points = mesh->GetControlPoints();
    std::vector<int32_t> tris_material_ids;
    tris_material_ids.resize(triangle_count);

    for (int32_t i = 0; i < triangle_count; ++i) {
        int32_t poly_size = mesh->GetPolygonSize(i);
        assert(poly_size == 3); //todo: triangulation

        for (int32_t j = 0; j < 3; j++) {
            int ctrl_point = mesh->GetPolygonVertex(i, j);
            xmdl_vertex vertex;
            memset(&vertex, 0, sizeof(xmdl_vertex));

            vertex.pos.x = (float) ctrl_points[ctrl_point][0];
            vertex.pos.y = (float) ctrl_points[ctrl_point][1];
            vertex.pos.z = (float) ctrl_points[ctrl_point][2];
            vertex.pos.z *= -1;//change

            read_normal(mesh, ctrl_point, i * 3 + j, &(vertex.normal));
            read_uv(mesh, ctrl_point, mesh->GetTextureUVIndex(i, j), 0, &(vertex.uv)); //1 uv set
            read_color(mesh, ctrl_point, i * 3 + j, &(vertex.color));
            e_mesh.vertices.push_back(vertex);
        }

    }

    read_triangle_material(mesh, triangle_count, tris_material_ids.data());
    if(triangle_count > 0) {

        //sort triangles to minimize material switching
        auto p = sort_permutation(tris_material_ids, [](int32_t const &a, int32_t const &b) { return 0; });
        apply_permutation_in_place_multiple(tris_material_ids, p, 1);
        apply_permutation_in_place_multiple(e_mesh.vertices, p, 3);

        xmdl_mesh_draw_call df_draw_call;
        df_draw_call.material_id = tris_material_ids[0];
        df_draw_call.vertices = 0;
        for (int32_t i = 0; i < tris_material_ids.size(); ++i) {
            //fprintf(stdout, "%i \n", tris_material_ids[i]);
            if (df_draw_call.material_id == tris_material_ids[i]) df_draw_call.vertices += 3;
            else {
                e_mesh.draw_calls.push_back(df_draw_call);
                df_draw_call.material_id = tris_material_ids[i];
                df_draw_call.vertices = 0;
            }
        }

        //fix material indexes
        //they are numbered 0-1 but are local to mesh...

        e_mesh.draw_calls.push_back(df_draw_call);

    }

    e_node->block_index = data->meshes.size();
    data->meshes.push_back(e_mesh);
}

void read_geometry_mat4(FbxNode* node, xmdl_mat* mat) {
    FbxAMatrix geometry;
    geometry.SetIdentity();
    if (node->GetNodeAttribute() != 0) {
        const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
        const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
        const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);
        geometry.SetT(lT);
        geometry.SetR(lR);
        geometry.SetS(lS);
    }

    //since it is column major we reverse
    for (uint32_t i = 0; i < 16; ++i) mat->data[i] = geometry.GetColumn(i / 4)[i % 4];
}

void read_normal(FbxMesh* mesh, int ctrl_point, int vertex_counter, xmdl_vec3* normal) {
    if (mesh->GetElementNormalCount() < 1)
        return;

    FbxGeometryElementNormal *element_normal = mesh->GetElementNormal(0);
    switch (element_normal->GetMappingMode()) {
        case FbxGeometryElement::eByControlPoint: {
            switch (element_normal->GetReferenceMode()) {
                case FbxGeometryElement::eDirect: {
                    normal->x = (float) element_normal->GetDirectArray().GetAt(ctrl_point)[0];
                    normal->y = (float) element_normal->GetDirectArray().GetAt(ctrl_point)[1];
                    normal->z = (float) element_normal->GetDirectArray().GetAt(ctrl_point)[2];
                }
                    break;

                case FbxGeometryElement::eIndexToDirect: {
                    int id = element_normal->GetIndexArray().GetAt(ctrl_point);
                    normal->x = (float) element_normal->GetDirectArray().GetAt(id)[0];
                    normal->y = (float) element_normal->GetDirectArray().GetAt(id)[1];
                    normal->z = (float) element_normal->GetDirectArray().GetAt(id)[2];
                }
                    break;

                default:
                    break;
            }
        }
            break;

        case FbxGeometryElement::eByPolygonVertex: {
            switch (element_normal->GetReferenceMode()) {
                case FbxGeometryElement::eDirect: {
                    normal->x = (float) element_normal->GetDirectArray().GetAt(vertex_counter)[0];
                    normal->y = (float) element_normal->GetDirectArray().GetAt(vertex_counter)[1];
                    normal->z = (float) element_normal->GetDirectArray().GetAt(vertex_counter)[2];
                }
                    break;

                case FbxGeometryElement::eIndexToDirect: {
                    int id = element_normal->GetIndexArray().GetAt(vertex_counter);
                    normal->x = (float) element_normal->GetDirectArray().GetAt(id)[0];
                    normal->y = (float) element_normal->GetDirectArray().GetAt(id)[1];
                    normal->z = (float) element_normal->GetDirectArray().GetAt(id)[2];
                }
                    break;

                default:
                    break;
            }
        }
            break;
    }

    normal->z *= -1;
}

void read_uv(FbxMesh* mesh, int ctrl_point, int tex_uv_id, int uvLayer, xmdl_vec2* uv) {
    if (uvLayer >= 2 || mesh->GetElementUVCount() <= uvLayer)
        return;

    FbxGeometryElementUV *vertex_uv = mesh->GetElementUV(uvLayer);

    switch (vertex_uv->GetMappingMode()) {
        case FbxGeometryElement::eByControlPoint: {
            switch (vertex_uv->GetReferenceMode()) {
                case FbxGeometryElement::eDirect: {
                    uv->x = (float) vertex_uv->GetDirectArray().GetAt(ctrl_point)[0];
                    uv->y = (float) vertex_uv->GetDirectArray().GetAt(ctrl_point)[1];
                }
                    break;

                case FbxGeometryElement::eIndexToDirect: {
                    int id = vertex_uv->GetIndexArray().GetAt(ctrl_point);
                    uv->x = (float) vertex_uv->GetDirectArray().GetAt(id)[0];
                    uv->y = (float) vertex_uv->GetDirectArray().GetAt(id)[1];
                }
                    break;
                default:
                    break;
            }
        }
            break;

        case FbxGeometryElement::eByPolygonVertex: {
            switch (vertex_uv->GetReferenceMode()) {
                case FbxGeometryElement::eDirect:
                case FbxGeometryElement::eIndexToDirect: {
                    uv->x = (float) vertex_uv->GetDirectArray().GetAt(tex_uv_id)[0];
                    uv->y = (float) vertex_uv->GetDirectArray().GetAt(tex_uv_id)[1];
                }
                    break;
                default:
                    break;
            }
        }
            break;
    }
    //uv->x = 1 - uv->x;
    uv->y = 1 - uv->y;
}

void read_color(FbxMesh* mesh , int ctrl_point , int vertex_counter , xmdl_vec3* color)
{
    if(mesh->GetElementVertexColorCount() < 1) {
        return;
    }

    FbxGeometryElementVertexColor* pVertexColor = mesh->GetElementVertexColor(0);
    switch(pVertexColor->GetMappingMode())
    {
        case FbxGeometryElement::eByControlPoint:
        {
            switch(pVertexColor->GetReferenceMode())
            {
                case FbxGeometryElement::eDirect:
                {
                    color->x = pVertexColor->GetDirectArray().GetAt(ctrl_point).mRed;
                    color->y = pVertexColor->GetDirectArray().GetAt(ctrl_point).mGreen;
                    color->z = pVertexColor->GetDirectArray().GetAt(ctrl_point).mBlue;
                }
                    break;
                case FbxGeometryElement::eIndexToDirect:
                {
                    int id = pVertexColor->GetIndexArray().GetAt(ctrl_point);
                    color->x = pVertexColor->GetDirectArray().GetAt(id).mRed;
                    color->y = pVertexColor->GetDirectArray().GetAt(id).mGreen;
                    color->z = pVertexColor->GetDirectArray().GetAt(id).mBlue;
                }
                    break;
                default:
                    break;
            }
        }
            break;
        case FbxGeometryElement::eByPolygonVertex:
        {
            switch (pVertexColor->GetReferenceMode())
            {
                case FbxGeometryElement::eDirect:
                {
                    color->x = pVertexColor->GetDirectArray().GetAt(vertex_counter).mRed;
                    color->y = pVertexColor->GetDirectArray().GetAt(vertex_counter).mGreen;
                    color->z = pVertexColor->GetDirectArray().GetAt(vertex_counter).mBlue;
                }
                    break;
                case FbxGeometryElement::eIndexToDirect:
                {
                    int id = pVertexColor->GetIndexArray().GetAt(vertex_counter);
                    color->x = pVertexColor->GetDirectArray().GetAt(id).mRed;
                    color->y = pVertexColor->GetDirectArray().GetAt(id).mGreen;
                    color->z = pVertexColor->GetDirectArray().GetAt(id).mBlue;
                }
                    break;
                default:
                    break;
            }
        }
            break;
    }
}

void read_triangle_material(FbxMesh* mesh , int tris_counter , int* triangle_material_index_array)
{
    FbxLayerElementArrayTemplate<int>* pMaterialIndices;
    FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
    if(mesh->GetElementMaterial())
    {
        pMaterialIndices = &(mesh->GetElementMaterial()->GetIndexArray());
        materialMappingMode = mesh->GetElementMaterial()->GetMappingMode();
        if(pMaterialIndices)
        {
            switch(materialMappingMode)
            {
                case FbxGeometryElement::eByPolygon:
                {
                    if(pMaterialIndices->GetCount() == tris_counter)
                    {
                        for(int triangleIndex = 0 ; triangleIndex < tris_counter ; ++triangleIndex)
                        {
                            int materialIndex = pMaterialIndices->GetAt(triangleIndex);
                            triangle_material_index_array[triangleIndex] = materialIndex;
                        }
                    }
                }
                    break;
                case FbxGeometryElement::eAllSame:
                {
                    int lMaterialIndex = pMaterialIndices->GetAt(0);
                    for(int triangleIndex = 0 ; triangleIndex < tris_counter ; ++triangleIndex)
                    {
                        int materialIndex = pMaterialIndices->GetAt(triangleIndex);
                        triangle_material_index_array[triangleIndex] = materialIndex;
                    }
                }
            }
        }
    }
}