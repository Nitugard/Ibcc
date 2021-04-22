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

#define MATERIAL_PROPERTIES 4

typedef struct scene_node {
    xmdl_node node;
} scene_node;

typedef struct scene_mesh_draw_call{
    uint32_t offset;
    uint32_t vertices;
    uint32_t material_id;
} scene_mesh_draw_call;

typedef struct scene_mesh{
    gfx_pipeline_handle pipeline_handle;
    uint32_t draw_calls_count;
    scene_mesh_draw_call* draw_calls;
    gfx_buffer_handle vertex_buffer;
    xmdl_mat model;
} scene_mesh;

typedef struct scene_texture{
    gfx_texture_handle gfx_handle;
} scene_texture;

typedef struct scene_material{
    xmdl_material mat; //todo!
} scene_material;

typedef struct scene_data{
    uint32_t meshes_count;
    scene_mesh* meshes;

    uint32_t textures_count;
    scene_texture* textures;

    uint32_t mats_count;
    scene_material* materials;

    uint32_t nodes_count;
    uint32_t* nodes;
} scene_data;

void scene_new_mesh_from_xmesh(gfx_shader_handle shader_handle, gfx_buffer_handle matrix_handle, struct scene_mesh* smesh, xmdl_mesh xmesh){
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
                    },
                    [uv_attr] = {
                            .buffer = smesh->vertex_buffer,
                            .offset = offsetof(xmdl_vertex, uv),
                            .stride = sizeof(struct xmdl_vertex),
                    }
            },

            //todo: remove uniforms from here since this MUST BE PER DRAW CALL
            .uniforms = {
                    {.name = "model", .offset = 0},
                    {.name = "diffuse", .offset = offsetof(xmdl_material, diffuse_color)},
            }
    };

    smesh->pipeline_handle = gfx_pipeline_create(&pip_desc);
    uint32_t offset = 0;
    smesh->draw_calls_count = xmesh.draw_call_count;
    smesh->draw_calls = OS_MALLOC(sizeof(scene_mesh_draw_call) * xmesh.draw_call_count);
    for(int32_t i=0; i<xmesh.draw_call_count; ++i)
    {
        smesh->draw_calls[i].vertices = xmesh.draw_calls[i].vertices;
        smesh->draw_calls[i].offset = offset;
        smesh->draw_calls[i].material_id = xmesh.draw_calls[i].material_id;
        offset += smesh->draw_calls[i].vertices;
    }
}

void scene_new_mat_from_xmat(scene_material* mat, xmdl_material xmat)
{
    mat->mat = xmat;
}

void scene_new_tex_from_xtex(scene_texture* tex, xmdl_texture xtex) {

    //todo: number of channels
    gfx_texture_desc desc = {.data = xtex.buffer,
                             .width = xtex.width,
                             .height = xtex.height,
                             .channels = 4};
    tex->gfx_handle = gfx_texture_create(&desc);
}

scene_handle scene_new(gfx_buffer_handle camera_handle, xmdl_data xmdl) {
    scene_handle handle = OS_MALLOC(sizeof(struct scene_data));

    handle->meshes_count = xmdl.mesh_count;
    handle->meshes = OS_MALLOC(sizeof(struct scene_mesh) * xmdl.mesh_count);

    handle->textures_count = xmdl.texture_count;
    handle->textures = OS_MALLOC(sizeof(struct scene_texture) * xmdl.texture_count);

    handle->mats_count = xmdl.material_count;
    handle->materials = OS_MALLOC(sizeof(struct scene_material) * xmdl.material_count);

    gfx_shader_handle sh_handle = gfx_shader_create(&unlit_shader_desc);

    for (uint32_t i = 0; i < handle->textures_count; ++i) {
        scene_new_tex_from_xtex(handle->textures + i, xmdl.textures[i]);
    }

    for (uint32_t i = 0; i < handle->mats_count; ++i) {
        scene_new_mat_from_xmat(handle->materials + i, xmdl.materials[i]);
    }

    for (uint32_t i = 0; i < handle->meshes_count; ++i) {
        scene_new_mesh_from_xmesh(sh_handle, camera_handle, handle->meshes + i, xmdl.meshes[i]);
    }
    return handle;
}

void scene_draw(scene_handle handle) {
    for(int i=0; i<handle->meshes_count; ++i) {
        gfx_pipeline_apply(handle->meshes[i].pipeline_handle);
        gfx_pipeline_uniforms_update(handle->meshes[i].pipeline_handle, handle->meshes[i].model.data, 0, 1);
        for(int32_t j=0; j<handle->meshes[i].draw_calls_count; ++j)
        {
            struct scene_material smat = handle->materials[handle->meshes[i].draw_calls[j].material_id];
            gfx_pipeline_uniforms_update(handle->meshes[i].pipeline_handle, &smat.mat, 1, 1);
            if(smat.mat.diffuse_tex_id != -1)
                gfx_texture_bind(handle->textures[smat.mat.diffuse_tex_id].gfx_handle);

            gfx_draw(GFX_TRIANGLES, handle->meshes[i].draw_calls[j].offset, handle->meshes[i].draw_calls[j].vertices);
        }
    }
}
