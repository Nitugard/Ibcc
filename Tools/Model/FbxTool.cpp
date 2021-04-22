/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#include <assert.h>
#include "FbxTool.h"
#include <fstream>
#include <Argparse.h>

int main(int argc, char** argv) {

    argparse::ArgumentParser program("FbxTool", "0.0.1");

    program.add_argument("filename")
    .help("Path of the file to be imported")
    .action([](const std::string& value) {
        return value;
    });

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        exit(0);
    }

    auto input = program.get<std::string>("filename");
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
    if (root == 0) {
        fprintf(stderr, "Could not find root node\n");
        return false;
    }

    process_node(root, data);

    //todo export test as well as binary
}

uint32_t process_node(FbxNode* node, xmdl_data_handle data) {
    FbxNodeAttribute *attribute = node->GetNodeAttribute();
    xmdl_node e_node = {};
    e_node.children_id = {};
    e_node.block_index = -1;
    e_node.block_type = XMDL_NODE;
    e_node.child_count = node->GetChildCount();
    //todo: transformations

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
                process_mesh(node, &e_node, data);
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
        uint32_t child_id = process_node(node->GetChild(i), data);
        if(child_id == -1) continue;
        data->nodes[node_id].children_id.push_back(child_id);
    }

    return node_id;
}

bool process_mesh(FbxNode *node, xmdl_node* e_node, xmdl_data_handle data) {
    FbxMesh *mesh = node->GetMesh();
    if (mesh == nullptr)
        return false;

    e_node->block_type = XMDL_MESH;
    xmdl_mesh e_mesh;
    e_mesh.material_id = -1;
    e_mesh.vertices_count = 0;
    e_mesh.vertices = {};
    int32_t triangle_count = mesh->GetPolygonCount();

    //control points is synonym for vertex
    FbxVector4* ctrl_points = mesh->GetControlPoints();

    for (int32_t i = 0; i < triangle_count; ++i)
    {
        int32_t poly_size = mesh->GetPolygonSize(i);
        assert(poly_size == 3); //todo: triangulation

        for (int32_t j = 0; j < 3; j++)
        {
            int ctrlPointIndex = mesh->GetPolygonVertex(i, j);
            xmdl_vertex vertex;
            memset(&vertex, 0, sizeof(xmdl_vertex));

            vertex.pos.x = (float)ctrl_points[ctrlPointIndex][0];
            vertex.pos.y = (float)ctrl_points[ctrlPointIndex][1];
            vertex.pos.z = (float)ctrl_points[ctrlPointIndex][2];

            read_normal(mesh, ctrlPointIndex, i*3 + j, &(vertex.normal));
            e_mesh.vertices.push_back(vertex);
        }
    }

    e_node->block_index = data->meshes.size();
    data->meshes.push_back(e_mesh);
}

void read_normal(FbxMesh* pMesh, int ctrlPointIndex, int vertexCounter, xmdl_vec3* pNormal) {
    if (pMesh->GetElementNormalCount() < 1)
        return;

    FbxGeometryElementNormal *element_normal = pMesh->GetElementNormal(0);
    switch (element_normal->GetMappingMode()) {
        case FbxGeometryElement::eByControlPoint: {
            switch (element_normal->GetReferenceMode()) {
                case FbxGeometryElement::eDirect: {
                    pNormal->x = (float) element_normal->GetDirectArray().GetAt(ctrlPointIndex)[0];
                    pNormal->y = (float) element_normal->GetDirectArray().GetAt(ctrlPointIndex)[1];
                    pNormal->z = (float) element_normal->GetDirectArray().GetAt(ctrlPointIndex)[2];
                }
                    break;

                case FbxGeometryElement::eIndexToDirect: {
                    int id = element_normal->GetIndexArray().GetAt(ctrlPointIndex);
                    pNormal->x = (float) element_normal->GetDirectArray().GetAt(id)[0];
                    pNormal->y = (float) element_normal->GetDirectArray().GetAt(id)[1];
                    pNormal->z = (float) element_normal->GetDirectArray().GetAt(id)[2];
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
                    pNormal->x = (float) element_normal->GetDirectArray().GetAt(vertexCounter)[0];
                    pNormal->y = (float) element_normal->GetDirectArray().GetAt(vertexCounter)[1];
                    pNormal->z = (float) element_normal->GetDirectArray().GetAt(vertexCounter)[2];
                }
                    break;

                case FbxGeometryElement::eIndexToDirect: {
                    int id = element_normal->GetIndexArray().GetAt(vertexCounter);
                    pNormal->x = (float) element_normal->GetDirectArray().GetAt(id)[0];
                    pNormal->y = (float) element_normal->GetDirectArray().GetAt(id)[1];
                    pNormal->z = (float) element_normal->GetDirectArray().GetAt(id)[2];
                }
                    break;

                default:
                    break;
            }
        }
            break;
    }

    pNormal->z *= -1;
}

