/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_OBJPARSER_H
#define IBC_OBJPARSER_H

#define MAXIMUM_MESH_NAME_SIZE 32
#define MAXIMUM_MAT_NAME_SIZE 128
#define MAXIMUM_LINE_SIZE 1024
#define MAXIMUM_TEX_NAME_SIZE 128

#define VERTEX_BUFFER_INITIAL_CAPACITY (1 << 18) //16kb
#define VERTEX_BUFFER_INCREMENT_CAPACITY (1 << 20) //128kb

#define FACE_BUFFER_INITIAL_CAPACITY (1<<18) //16kb
#define FACE_BUFFER_INCREMENT_CAPACITY (1 << 20) //128kb

#include <malloc.h>
#include <assert.h>

typedef struct vec3_t{
    float x, y, z;
} vec3_t;

typedef struct vec2_t{
    float x, y;
} vec2_t;


typedef struct face_t {
    uint32_t vert_id[3];
    uint32_t norm_id[3];
    uint32_t uv_id[3];
} face_t;

typedef struct mesh_t {
    char name[MAXIMUM_MESH_NAME_SIZE];
    char mat_name[MAXIMUM_MAT_NAME_SIZE];

    uint32_t vertices_length;
    uint32_t normals_length;
    uint32_t uvs_length;

    vec3_t* vertices;
    vec3_t* normals;
    vec2_t* uvs;

    face_t* faces;
    uint32_t faces_length;
} mesh_t;

typedef struct mesh_mat_t{

    char name[MAXIMUM_MAT_NAME_SIZE];
    float ns; //!specular highlights [0, 1000]
    float ni; //!index of refraction [0.001, 10], A value of 1.0 means that light does not bend as it passes through an object.
    vec3_t ka; //!ambient color
    vec3_t kd; //!diffuse color
    vec3_t ks; //!specular color
    float d; //!dissolve
    char tex[MAXIMUM_TEX_NAME_SIZE];
} mesh_mat_t;

typedef struct iterator_t{
    const char* current;
    int32_t offset;
} iterator_t;

bool str_cmp_len_1(char const* a, char b) {
    return *a == b;
}

bool str_cmp_len_2(char const* a, const char* b) {
    return *(int16_t *) a == *(int16_t *) b;
}

bool str_cmp_len_4(char const* a, const char* b, uint32_t length) {
    int32_t mask = (1 << length) - 1;
    return (mask & (*(int32_t *) a)) == ((*(int32_t *) b) & mask);
}

bool str_cmp_len_8(char const* a, const char* b, uint32_t length) {
    int32_t mask = (1 << length) - 1;
    return (mask & (*(int64_t *) a)) == ((*(int64_t *) b) & mask);
}

int32_t line_length_copy(const char* current, char* buff) {
    uint32_t i = 0;

    while (*current != '\0') {
        if (current[i] == '\n')
            return i;
        buff[i] = current[i];
        ++i;
    }

    return i;
}


//! data must be triangulated
void parse_obj(const char *data, const char* material, mesh_t **meshes, uint32_t *meshes_count)
{
    char linebuffer[1024];
    int32_t row = 0;
    uint32_t mesh_count = 0;
    const char *current = data;
    struct mesh_t *mesh = 0;
    uint32_t buffer_size = 0;


    uint32_t meshes_alloc = 4;
    *meshes = malloc(sizeof(mesh_t) * meshes_alloc);

    do {
        if (row != 0) ++current; //skip \n character
        int32_t ll = line_length_copy(current, linebuffer);
        linebuffer[ll] = '\0';
        if (str_cmp_len_2(current, "g ")) {
            //todo
            assert(false && "Currently obj file does not support groups!");
        }
        else if (str_cmp_len_2(current, "o ")) {
            //object
            if(meshes_alloc == mesh_count){
                meshes_alloc *= 2;
                *meshes = malloc(sizeof(mesh_t) * meshes_alloc);
            }
            mesh = *meshes + mesh_count;
            memset(mesh, 0, sizeof(mesh_t));
            ++mesh_count;
            sprintf(mesh->name, "%.*s", ll, &linebuffer[0] + 2);
            current += ll;
        } else if (str_cmp_len_2(linebuffer, "v ")) {
            //vertex
            assert(mesh != 0);
            if (mesh->vertices == 0) {
                buffer_size = VERTEX_BUFFER_INITIAL_CAPACITY;
                mesh->vertices = malloc(sizeof(struct vec3_t) * buffer_size);
                mesh->vertices_length = 0;
            }
            if (mesh->vertices_length >= buffer_size) {
                buffer_size += VERTEX_BUFFER_INCREMENT_CAPACITY;
                mesh->vertices = realloc(mesh->vertices, sizeof(struct vec3_t) * buffer_size);
            }

            sscanf(&linebuffer[0] + 2, "%f %f %f",
                   (float *) (mesh->vertices + mesh->vertices_length) + 0,
                   (float *) (mesh->vertices + mesh->vertices_length) + 1,
                   (float *) (mesh->vertices + mesh->vertices_length) + 2);
            ++mesh->vertices_length;
            current += ll;
        } else if (str_cmp_len_2(current, "vt")) {

        } else if (str_cmp_len_2(current, "vn")) {
            //normal
            assert(mesh != 0);
            if (mesh->normals == 0) {
                if (mesh->vertices_length == 0) {
                    buffer_size = VERTEX_BUFFER_INITIAL_CAPACITY;
                } else {
                    buffer_size = mesh->vertices_length;
                }

                mesh->normals = malloc(sizeof(struct vec3_t) * buffer_size);
                mesh->normals_length = 0;
            }
            if (mesh->normals_length >= buffer_size) {
                buffer_size += VERTEX_BUFFER_INCREMENT_CAPACITY;
                mesh->normals = realloc(mesh->normals, sizeof(struct vec3_t) * buffer_size);
            }

            sscanf(&linebuffer[0] + 3, "%f %f %f",
                   (float *) (mesh->normals + mesh->normals_length) + 0,
                   (float *) (mesh->normals + mesh->normals_length) + 1,
                   (float *) (mesh->normals + mesh->normals_length) + 2);
            ++mesh->normals_length;
            current += ll;
        } else if (str_cmp_len_2(current, "f ")) {
            assert(mesh != 0);
            if (mesh->faces == 0) {
                buffer_size = FACE_BUFFER_INITIAL_CAPACITY;
                mesh->faces = malloc(sizeof(struct face_t) * buffer_size);
                mesh->faces_length = 0;
            }
            if (mesh->faces_length >= buffer_size) {
                buffer_size += FACE_BUFFER_INCREMENT_CAPACITY;
                mesh->faces = realloc(mesh->faces, sizeof(struct face_t) * buffer_size);
            }

            face_t *face = mesh->faces + mesh->faces_length;
            sscanf(&linebuffer[0] + 2, "%i/%i/%i %i/%i/%i %i/%i/%i",
                   face->vert_id, face->uv_id, face->norm_id,
                   face->vert_id + 1, face->uv_id + 1, face->norm_id + 1,
                   face->vert_id + 2, face->uv_id + 2, face->norm_id + 2);
            for(int i=0; i<3; ++i) {
                face->vert_id[i] -= 1;
                face->norm_id[i] -= 1;

            }
            current += ll;

            ++mesh->faces_length;
        }
        else if(str_cmp_len_8("usemtl", current, 6)) {
            sscanf(&linebuffer[0] + 7, "%s", mesh->mat_name);
            current += ll;
        }
        else {
            current += ll;
        }
        ++row;
    } while (*current != '\0');

    //load material
    if(material != 0) {
        row = 0;
        current = material;
        mesh_mat_t *mesh_mat = 0;
        uint32_t mat_count = 0;

        uint32_t materials_alloc = 4;
        mesh_mat_t *materials = malloc(sizeof(mesh_mat_t) * materials_alloc);

        do {
            if (row != 0) ++current; //skip \n character
            int32_t ll = line_length_copy(current, linebuffer);
            linebuffer[ll] = '\0';
            if (str_cmp_len_8(current, "newmtl", 6)) {
                if (materials_alloc == mat_count) {
                    materials_alloc *= 2;
                    materials = malloc(sizeof(mesh_mat_t) * materials_alloc);
                }

                mesh_mat = materials + mat_count;
                memset(mesh_mat, 0, sizeof(mesh_mat_t));
                ++mat_count;
                sprintf(mesh_mat->name, "%.*s", ll, &linebuffer[0] + 7);
            } else if (str_cmp_len_2(current, "Ns")) {
                sscanf(&linebuffer[0] + 3, "%f", &(mesh_mat->ns));
            } else if (str_cmp_len_2(current, "Ka")) {
                sscanf(&linebuffer[0] + 3, "%f %f %f", &(mesh_mat->ka.x), &(mesh_mat->ka.y), &(mesh_mat->ka.z));
            } else if (str_cmp_len_2(current, "Kd")) {
                sscanf(&linebuffer[0] + 3, "%f %f %f", &(mesh_mat->kd.x), &(mesh_mat->kd.y), &(mesh_mat->kd.z));
            } else if (str_cmp_len_2(current, "Ks")) {
                sscanf(&linebuffer[0] + 3, "%f %f %f", &(mesh_mat->ks.x), &(mesh_mat->ks.y), &(mesh_mat->ks.z));
            } else if (str_cmp_len_2(current, "Ke")) {
            } else if (str_cmp_len_2(current, "Ni")) {
                sscanf(&linebuffer[0] + 3, "%f", &(mesh_mat->ni));
            } else if (str_cmp_len_2(current, "d ")) {
                sscanf(&linebuffer[0] + 2, "%f", &(mesh_mat->d));
            }


            current += ll;
            ++row;
        } while (*current != '\0');
    }

    *meshes_count = mesh_count;
}

void free_obj(mesh_t* meshes, uint32_t count) {
    for(uint32_t i=0; i<count; ++i)
    {
        free(meshes[i].vertices);
        free(meshes[i].normals);
        free(meshes[i].uvs);
        free(meshes[i].faces);

    }
    free(meshes);
}

#endif //IBC_OBJPARSER_H
