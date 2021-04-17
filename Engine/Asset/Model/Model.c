
#include "Model.h"

#include <Os/Log.h>
#include <Os/Allocator.h>
#include <Asset/Asset.h>
#include <Os/File.h>
#include <Os/Plugin.h>
#include <Containers/String.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ObjParser.h"

static void cross(float *N, const float *v0, const float *v1, const float *v2) {
    float v10[3];
    float v20[3];
    float len_squared;

    v10[0] = v1[0] - v0[0];
    v10[1] = v1[1] - v0[1];
    v10[2] = v1[2] - v0[2];

    v20[0] = v2[0] - v0[0];
    v20[1] = v2[1] - v0[1];
    v20[2] = v2[2] - v0[2];

    N[0] = v20[1] * v10[2] - v20[2] * v10[1];
    N[1] = v20[2] * v10[0] - v20[0] * v10[2];
    N[2] = v20[0] * v10[1] - v20[1] * v10[0];

    len_squared = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
    if (len_squared > 0.0f) {
        float rcplen = 1.0f / (float) sqrt((double) len_squared);
        N[0] *= rcplen;
        N[1] *= rcplen;
    }
}


asset_hndl asset_on_load_model(char* filename) {

    uint32_t length;
    char *data = file_mmap(filename, &length);
    if (data == 0) return 0;

    mesh_t meshes[4];
    uint32_t meshes_count;
    parse(data, meshes, &meshes_count);

    mdl_data_handle handle = OS_MALLOC(sizeof (struct mdl_data));

    handle->stride = sizeof(vec3_t) + sizeof(struct vec3_t);
    handle->buffer_size = handle->stride * 3 * meshes[0].faces_length;
    handle->buffer = OS_MALLOC(handle->buffer_size);

    for(unsigned int i=0; i<meshes[0].faces_length; ++i) {
        struct face_t face = meshes[0].faces[i];
        struct vec3_t* vbuf = (struct vec3_t *) handle->buffer;

        *(vbuf + (i * 6) + 0 ) = meshes[0].vertices[face.vert_id[0]];
        *(vbuf + (i * 6) + 2 ) = meshes[0].vertices[face.vert_id[1]];
        *(vbuf + (i * 6) + 4 ) = meshes[0].vertices[face.vert_id[2]];

        *(vbuf + (i * 6) + 1 ) = meshes[0].normals[face.norm_id[0]];
        *(vbuf + (i * 6) + 3 ) = meshes[0].normals[face.norm_id[1]];
        *(vbuf + (i * 6) + 5 ) = meshes[0].normals[face.norm_id[2]];
    }

    //free();


    return handle;
}

void asset_on_unload_model(asset_hndl hndl)
{
    mdl_data_handle mdl = (mdl_data_handle) hndl;
    OS_FREE(mdl->buffer);
    OS_FREE(mdl);

}

void init_model_asset(plg_info const* info) {
    asset_register_desc jpg_desc = {
            .extension = "obj",
            .asset_on_load = asset_on_load_model,
            .asset_on_unload = asset_on_unload_model
    };
    asset_register(&jpg_desc);
}

