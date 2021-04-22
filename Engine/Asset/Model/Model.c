/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#define XMDL_IMPLEMENTATION
#include "Model.h"

#include <Os/Allocator.h>
#include <Asset/Asset.h>
#include <Os/File.h>

typedef struct r_buf{
    void* buf;
    int32_t pos;
    int32_t size;
} r_buf;



void read_next_noalloc(void* dest, struct r_buf* buf, int32_t size) {
    os_memcpy(dest, buf->buf + buf->pos, size);
    buf->pos += size;
}

void read_next(void** dest, struct r_buf* buf, int32_t size) {
    *dest = OS_MALLOC(size);
    os_memcpy(*dest, buf->buf + buf->pos, size);
    buf->pos += size;
}

int32_t read_int32(struct r_buf* buf) {
    int32_t res = *(int32_t *) (buf->buf + buf->pos);
    buf->pos += sizeof(int32_t);
    return res;
}

xmdl_data_handle read(void* buffer, uint32_t size) {
    xmdl_data_handle data = OS_MALLOC(sizeof(struct xmdl_data));
    r_buf rbuf = {.size = size, .pos = 0, .buf = buffer};
    data->version = read_int32(&rbuf);
    data->mesh_count = read_int32(&rbuf);
    data->material_count = read_int32(&rbuf);
    data->texture_count = read_int32(&rbuf);
    data->node_count = read_int32(&rbuf);

    //reading meshes
    data->meshes = OS_MALLOC(sizeof(struct xmdl_mesh) * data->mesh_count);
    for (int32_t i = 0; i < data->mesh_count; ++i) {
        struct xmdl_mesh *mesh = data->meshes + i;
        mesh->vertices_count = read_int32(&rbuf);
        mesh->material_id = read_int32(&rbuf);
        read_next((void **) &(mesh->vertices), &rbuf, sizeof(struct xmdl_vertex) * mesh->vertices_count);
    }

    //reading nodes
    data->nodes = OS_MALLOC(sizeof(struct xmdl_node) * data->node_count);
    for (int32_t i = 0; i < data->node_count; ++i) {
        struct xmdl_node *node = data->nodes + i;
        read_next_noalloc((void *) (node->tr_world.data), &rbuf, sizeof(struct xmdl_mat));
        read_next_noalloc((void *) (node->tr_parent.data), &rbuf, sizeof(struct xmdl_mat));
        node->block_type = read_int32(&rbuf);
        node->block_index = read_int32(&rbuf);
        node->child_count = read_int32(&rbuf);
        read_next((void **) &(node->children_id), &rbuf, sizeof(int32_t) * node->child_count);
    }

    return data;
}

asset_hndl asset_on_load_model(char* filename) {

    xmdl_data_handle data;
    uint32_t len;
    char *buffer = file_mmap(filename, &len);
    data = read(buffer, len);

    return (asset_hndl) data;
}

void asset_on_unload_model(asset_hndl hndl)
{
    //TODO: DELETE
}

void init_model_asset() {
    asset_register_desc jpg_desc = {
            .extension = "xmdl",
            .asset_on_load = asset_on_load_model,
            .asset_on_unload = asset_on_unload_model
    };
    asset_register(&jpg_desc);
}

xmdl_mat xmdl_mat_identity() {
    xmdl_mat result = {.data={1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1}};
    return result;
}
