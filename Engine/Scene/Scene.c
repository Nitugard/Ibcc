/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include <stddef.h>
#include "Os/Allocator.h"
#include "Scene.h"
#include "Graphics/Graphics.h"
#include "Shaders/BlinnPhongShader.h"

typedef struct scene_node {
    xmdl_node node;
} scene_node;

typedef struct scene_mesh{
    gfx_pipeline_handle pipeline_handle;
    gfx_buffer_handle vertex_buffer;
    uint32_t vertices;
    xmdl_mat model;
} scene_mesh;

typedef struct scene_data{
    uint32_t meshes_count;
    scene_mesh* meshes;

    uint32_t nodes_count;
    uint32_t* nodes;
} scene_data;

void scene_new_mesh(gfx_shader_handle shader_handle, gfx_buffer_handle matrix_handle, struct scene_mesh* smesh, xmdl_mesh xmesh){
    gfx_buffer_desc buffer_desc = {
            .data = xmesh.vertices,
            .size = sizeof(struct xmdl_vertex) * xmesh.vertices_count,
            .type = VERTEX,
            .update_mode = STATIC_DRAW
    };
    smesh->vertex_buffer = gfx_buffer_create(&buffer_desc);
    smesh->model = xmdl_mat_identity();
    gfx_pipeline_desc pip_desc = {
            .shader = shader_handle,
            .index_buffer = 0,
            .uniform_blocks[0] = {
                    .name = "matrices",
                    .buffer = matrix_handle
            },
            .attrs = {
                    [position_attr] = {
                            .buffer = smesh->vertex_buffer,
                            .offset = 0,
                            .stride = sizeof(struct xmdl_vertex),
                    },
                    [normal_attr] = {
                            .buffer = smesh->vertex_buffer,
                            .offset = offsetof(xmdl_vertex, normal),
                            .stride = sizeof(struct xmdl_vertex),
                    }
            },
            .uniforms = {
                    [0] = {.name = "model", .buffer = smesh->model.data, .offset = 0}
            }
    };

    smesh->pipeline_handle = gfx_pipeline_create(&pip_desc);
    smesh->vertices = xmesh.vertices_count;
}

scene_handle scene_new(gfx_buffer_handle camera_handle, xmdl_data xmdl) {
    scene_handle handle = OS_MALLOC(sizeof(struct scene_data));

    handle->meshes_count = xmdl.mesh_count;
    handle->meshes = OS_MALLOC(sizeof(struct scene_mesh) * xmdl.mesh_count);

    gfx_shader_handle sh_handle = gfx_shader_create(&unlit_shader_desc);

    for (uint32_t i = 0; i < handle->meshes_count; ++i) {
        scene_new_mesh(sh_handle, camera_handle, handle->meshes + i, xmdl.meshes[i]);
    }
    return handle;
}

void scene_draw(scene_handle handle) {
    for(int i=0; i<handle->meshes_count; ++i)
    {
        gfx_pipeline_apply(handle->meshes[i].pipeline_handle);
        gfx_pipeline_uniforms_update(handle->meshes[i].pipeline_handle, 0, 1);
        gfx_draw(GFX_TRIANGLES, 0, handle->meshes[i].vertices);
    }
}
