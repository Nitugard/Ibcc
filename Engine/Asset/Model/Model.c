
#include "Model.h"

#include <Os/Allocator.h>
#include <Asset/Asset.h>
#include <Os/File.h>
#include "ObjParser.h"


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
    handle->buffer_size = sizeof(struct vertex_t) * total_vertices;
    handle->buffer = OS_MALLOC(handle->buffer_size);
    vertex_t *vbuf = (vertex_t *) handle->buffer;
    os_memset(vbuf, 0, handle->buffer_size);
    mesh_t mesh;
    face_t face;
    for(uint32_t j=0; j<meshes_count; ++j) {
        mesh = meshes[j];
        for (uint32_t i = 0; i < mesh.faces_length; ++i) {
            face = mesh.faces[i];
            for(uint32_t k=0; k<3; ++k) {
                if (mesh.uvs_length != 0) os_memcpy(vbuf->uv, (&mesh.uvs[face.uv_id[k]]), sizeof(struct vec2_t));
                if (mesh.normals_length != 0) os_memcpy(vbuf->normal, (&mesh.normals[face.norm_id[k]]), sizeof(struct vec3_t));
                if (mesh.normals_length != 0) os_memcpy(vbuf->color, (&mesh.normals[face.norm_id[k]]), sizeof(struct vec3_t));
                if (mesh.vertices_length != 0) os_memcpy(vbuf->pos, (&mesh.vertices[face.vert_id[k]]), sizeof(struct vec3_t));

                vbuf++;
            }
        }
    }

    free_obj(meshes, meshes_count);
    return (asset_hndl) handle;
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

