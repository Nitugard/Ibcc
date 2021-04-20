
#include "Model.h"

#include <Os/Log.h>
#include <Os/Allocator.h>
#include <Asset/Asset.h>
#include <Os/File.h>
#include <Containers/String.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ObjParser.h"

#define MDL_ELEMENT_SIZE 4

#define MDL_STRIDE_POS 3
#define MDL_STRIDE_COLOR 3
#define MDL_STRIDE_UV 2
#define MDL_STRIDE_NORMAL 3

#define MDL_OFFSET_POS 0
#define MDL_OFFSET_COLOR 3
#define MDL_OFFSET_UV 6
#define MDL_OFFSET_NORMAL 8

#define MDL_EL_SIZE(count) (MDL_ELEMENT_SIZE * (count))
#define MDL_STRIDE (MDL_STRIDE_POS + MDL_STRIDE_COLOR + MDL_STRIDE_UV + MDL_STRIDE_NORMAL)
#define MDL_STRIDE_SIZE MDL_EL_SIZE(MDL_STRIDE)

void set_vertex_data(float* pos, float* color, float* uv, float* normal, float* buffer)
{
    if(pos != 0) os_memcpy(buffer + MDL_OFFSET_POS, pos, MDL_EL_SIZE(MDL_STRIDE_POS));
    if(color != 0) os_memcpy(buffer + MDL_OFFSET_COLOR, color, MDL_EL_SIZE(MDL_STRIDE_COLOR));
    if(uv != 0) os_memcpy(buffer + MDL_OFFSET_UV, uv, MDL_EL_SIZE(MDL_STRIDE_UV));
    if(normal != 0) os_memcpy(buffer + MDL_OFFSET_NORMAL, normal, MDL_EL_SIZE(MDL_STRIDE_NORMAL));

}


asset_hndl asset_on_load_model(char* filename) {

    uint32_t length;
    char *data = file_mmap(filename, &length);
    if (data == 0) return 0;

    mesh_t* meshes;
    uint32_t meshes_count;
    parse_obj(data, 0, &meshes, &meshes_count);
    mdl_data_handle handle = OS_MALLOC(sizeof (struct mdl_data));

    uint32_t total_vertices = 0;
    for(uint32_t i=0; i<meshes_count; ++i) {
        total_vertices += meshes[i].faces_length * 3;
    }

    handle->vertices = total_vertices;
    handle->buffer_size = MDL_STRIDE_SIZE * total_vertices;
    handle->buffer = OS_MALLOC(handle->buffer_size);
    float *vbuf = (float *) handle->buffer;
    float *uvs =0, *normals =0, *colors=0, *pos = 0;
    mesh_t mesh;
    face_t face;
    for(uint32_t j=0; j<meshes_count; ++j) {
        mesh = meshes[j];
        for (uint32_t i = 0; i < mesh.faces_length; ++i) {
            face = mesh.faces[i];
            for(uint32_t k=0; k<3; ++k) {
                if(mesh.uvs_length != 0) uvs = (float *) (&mesh.uvs[face.uv_id[k]]);
                if(mesh.normals_length != 0) normals = (float *) (&mesh.normals[face.norm_id[k]]);
                if(mesh.normals_length != 0) colors =(float *) (&mesh.normals[face.norm_id[k]]);
                if(mesh.vertices_length != 0) pos =(float *) (&mesh.vertices[face.vert_id[k]]);

                set_vertex_data(pos, colors, uvs, normals, (vbuf + (i * 3 * MDL_STRIDE) + (k * MDL_STRIDE)));
            }
        }
    }

    free_obj(meshes, meshes_count);
    return handle;
}

void asset_on_unload_model(asset_hndl hndl)
{
    mdl_data_handle mdl = (mdl_data_handle) hndl;
    OS_FREE(mdl->buffer);
    OS_FREE(mdl);
}

void init_model_asset() {
    asset_register_desc jpg_desc = {
            .extension = "obj",
            .asset_on_load = asset_on_load_model,
            .asset_on_unload = asset_on_unload_model
    };
    asset_register(&jpg_desc);
}

