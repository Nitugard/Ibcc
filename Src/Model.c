/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Model.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "Allocator.h"
#include "GlMath.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "stb_image.h"

#ifndef CORE_ASSERT
#include "assert.h"
#define CORE_ASSERT(e) assert(e)
#endif

/*
 * Known limitations:
 * 1. Only 1 uv channel per primitive supported
 * 2. Only 8 textures currently supported
 * 3. Only pbr metallic roughness material supported
 * 4. No skinning
 * 5. ?
 */


static bool mdl_read_entire_file(const char* path, void** out_buffer, size_t* out_size)
{
    /*
     * Reads the whole glTF file into memory.
     * This prevents crashes when the file path is wrong, the file is empty,
     * or the read is incomplete.
     */
    if (out_buffer == 0 || out_size == 0) {
        fprintf(stderr, "- mdl_read_entire_file failed: output pointers are null\n");
        return false;
    }

    *out_buffer = 0;
    *out_size = 0;

    if (path == 0 || path[0] == '\0') {
        fprintf(stderr, "- mdl_read_entire_file failed: path is null or empty\n");
        return false;
    }

    FILE* file = fopen(path, "rb");
    if (file == 0) {
        fprintf(stderr, "- mdl_read_entire_file failed: could not open '%s'\n", path);
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "- mdl_read_entire_file failed: fseek end failed for '%s'\n", path);
        fclose(file);
        return false;
    }

    long file_size = ftell(file);
    if (file_size <= 0) {
        fprintf(stderr, "- mdl_read_entire_file failed: invalid file size for '%s'\n", path);
        fclose(file);
        return false;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "- mdl_read_entire_file failed: fseek start failed for '%s'\n", path);
        fclose(file);
        return false;
    }

    void* buffer = OS_MALLOC((size_t)file_size);
    if (buffer == 0) {
        fprintf(stderr, "- mdl_read_entire_file failed: allocation failed for '%s'\n", path);
        fclose(file);
        return false;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "- mdl_read_entire_file failed: incomplete read for '%s'\n", path);
        OS_FREE(buffer);
        return false;
    }

    *out_buffer = buffer;
    *out_size = (size_t)file_size;
    return true;
}

mdl_handle mdl_load(const char* path) {
    bool verbose = false;

    printf("Loading model %s\n", path != 0 ? path : "<null>");

    void* buffer = 0;
    size_t size = 0;

    if (!mdl_read_entire_file(path, &buffer, &size)) {
        return 0;
    }

    cgltf_options options = {0};
    cgltf_data *data = NULL;

    cgltf_result result = cgltf_parse(&options, buffer, size, &data);
    if (result != cgltf_result_success) {
        fprintf(stderr, "- Gltf parse failed %i\n", result);
        OS_FREE(buffer);
        return 0;
    }

    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) {
        fprintf(stderr, "- Gltf buffer loading failed %i\n", result);
        cgltf_free(data);
        OS_FREE(buffer);
        return 0;
    }

    result = cgltf_validate(data);
    if (result != cgltf_result_success) {
        fprintf(stderr, "- Gltf validation warning/error %i\n", result);
        /*
         * For development you may continue after validation warnings.
         * For production/import pipeline, prefer returning 0 here.
         */
    }

    mdl_handle handle = OS_MALLOC(sizeof(mdl_data));
    if (handle == 0) {
        fprintf(stderr, "- Model allocation failed\n");
        cgltf_free(data);
        OS_FREE(buffer);
        return 0;
    }

    os_memset(handle, 0, sizeof(mdl_data));

    /*
     * Loading of the scenes. Todo.
     */

    /*
     * Loading of the nodes.
     */

    if (verbose) printf("- Nodes count: %llu\n", (unsigned long long)data->nodes_count);

    handle->nodes_count = data->nodes_count;
    handle->nodes = OS_MALLOC(sizeof(struct mdl_node) * data->nodes_count);
    os_memset(handle->nodes, 0, sizeof(struct mdl_node) * data->nodes_count);

    for (int32_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node *cnode = data->nodes + i;
        mdl_node *node = handle->nodes + i;
        node->mesh_index = node->light_index = node->camera_index = -1;
        if (cnode->name != 0) {
            node->name = OS_MALLOC(strlen(cnode->name) + 1);
            os_memcpy(node->name, cnode->name, strlen(cnode->name) + 1);
        } else {
            node->name = 0;
        }
        if (verbose) printf("- - Loading node: %s\n", node->name != 0 ? node->name : "<unnamed>");

        if (cnode->has_matrix) {
            fprintf(stderr, "- - - Matrix is currently not supported\n");
        } else {

            os_memcpy(node->local_pos, gl_vec3_new(0,0,0).data, sizeof(float) * 3);
            os_memcpy(node->local_scale, gl_vec3_new(1,1,1).data, sizeof(float) * 3);
            os_memcpy(node->local_rot, gl_vec4_new(0,0,0,1).data, sizeof(float) * 4);

            if (cnode->has_translation) {
                os_memcpy(node->local_pos, cnode->translation, sizeof(float) * 3);
                printf("- - - Translation %f %f %f\n", cnode->translation[0], cnode->translation[1], cnode->translation[2]);
            }
            if (cnode->has_rotation) {
                os_memcpy(node->local_rot, cnode->rotation, sizeof(float) * 4);
                printf("- - - Rotation %f %f %f %f\n", cnode->rotation[0], cnode->rotation[1], cnode->rotation[2], cnode->rotation[3]);
            }
            if (cnode->has_scale) {
                os_memcpy(node->local_scale, cnode->scale, sizeof(float) * 3);
                printf("- - - Scale %f %f %f\n", cnode->scale[0], cnode->scale[1], cnode->scale[2]);
            }
        }

        //load children
        if (verbose) printf("- - - Children count: %llu\n", (unsigned long long)cnode->children_count);
        node->children_count = cnode->children_count;
        if (cnode->children_count > 0) {
            node->children_id = OS_MALLOC(sizeof(uint32_t) * cnode->children_count);
            for (int32_t j = 0; j < cnode->children_count; ++j) {
                cgltf_node *child_cnode = cnode->children[j];
                int32_t child_index = -1;
                for (int32_t k = 0; k < data->nodes_count; ++k) {
                    if (data->nodes + k == child_cnode) {
                        child_index = k;
                        break;
                    }
                }
                if (child_index == -1) {
                    printf("- - - - Child node index could not be found!");
                }
                else {
                    node->children_id[j] = child_index;
                    if (verbose) printf("- - - - Added child: %s, index: %i\n", child_cnode->name != 0 ? child_cnode->name : "<unnamed>", child_index);
                }
            }
        }

        //update node parent index
        node->parent_id = -1;
        for (int32_t k = 0; k < data->nodes_count; ++k) {
            if (data->nodes + k == cnode->parent) {
                node->parent_id = k;
                break;
            }
        }

        if(node->parent_id != -1) {
            printf("- - - - Parent index: %i, name: %s\n", node->parent_id, data->nodes[node->parent_id].name != 0 ? data->nodes[node->parent_id].name : "<unnamed>");
        }

        //associate node with mesh
        if (cnode->mesh) {
            int32_t mesh_index = -1;
            for (int32_t j = 0; j < data->meshes_count; ++j) {
                if (data->meshes + j == cnode->mesh) {
                    mesh_index = j;
                    break;
                }
            }
            if (mesh_index == -1) { printf("- - - Node mesh index could not be found!"); }
            else {
                node->mesh_index = mesh_index;
                if (verbose) printf("- - - Node mesh: %s, index: %i\n", cnode->mesh->name != 0 ? cnode->mesh->name : "<unnamed>", mesh_index);
            }
        }

        //associate node with camera
        if (cnode->camera) {
            int32_t camera_index = -1;
            for (int32_t j = 0; j < data->cameras_count; ++j) {
                if (data->cameras + j == cnode->camera) {
                    camera_index = j;
                    break;
                }
            }
            if (camera_index == -1) { printf("- - - Node camera index could not be found!"); }
            else {
                node->camera_index = camera_index;
                if (verbose) printf("- - - Node camera: %s, index: %i\n", cnode->camera->name != 0 ? cnode->camera->name : "<unnamed>", camera_index);
            }
        }

        //associate node with light
        if (cnode->light) {
            int32_t light_index = -1;
            for (int32_t j = 0; j < data->lights_count; ++j) {
                if (data->lights + j == cnode->light) {
                    light_index = j;
                    break;
                }
            }
            if (light_index == -1) { printf("- - - Node light index could not be found!"); }
            else {
                node->light_index = light_index;
                if (verbose) printf("- - - Node light: %s, index: %i\n", cnode->light->name != 0 ? cnode->light->name : "<unnamed>", light_index);
            }
        }
    }

    /*
     * Loading of the cameras.
     */
    handle->cameras = OS_MALLOC(sizeof(struct mdl_camera) * data->cameras_count);
    handle->cameras_count = data->cameras_count;
    os_memset(handle->cameras, 0, sizeof(struct mdl_camera) * data->cameras_count);
    if (verbose) printf("- Cameras count: %llu\n", (unsigned long long)data->cameras_count);

    for (int32_t i = 0; i < data->cameras_count; ++i) {
        struct cgltf_camera *ccamera = data->cameras + i;
        struct mdl_camera *camera = handle->cameras + i;

        if (ccamera->name != 0) {
            camera->name = OS_MALLOC(strlen(ccamera->name) + 1);
            os_memcpy(camera->name, ccamera->name, strlen(ccamera->name) + 1);
            if (verbose) printf("- - Loading camera: %s\n", camera->name);
        } else {
            if (verbose) printf("- - Loading unnamed camera\n");
            camera->name = 0;
        }

        switch(ccamera->type) {
            case cgltf_camera_type_invalid: fprintf(stderr, "- - - Invalid camera type\n");
                break;
            case cgltf_camera_type_perspective:
                camera->ortographic = false;
                printf("- - - Camera perspective\n");
                camera->fov = gl_rad2deg(ccamera->data.perspective.yfov);
                if(ccamera->data.perspective.has_zfar)
                    camera->zfar = ccamera->data.perspective.zfar;
                else
                    camera->zfar = DEFAULT_ZFAR;
                camera->znear = ccamera->data.perspective.znear;
                printf("- - - Fov %f\n", camera->fov);
                printf("- - - ZFar %f\n", camera->zfar);
                printf("- - - ZNear %f\n", camera->znear);
                break;
            case cgltf_camera_type_orthographic:
                camera->ortographic = true;
                camera->znear = ccamera->data.orthographic.znear;
                camera->zfar = ccamera->data.orthographic.zfar;
                camera->xmag = ccamera->data.orthographic.xmag;
                camera->ymag = ccamera->data.orthographic.ymag;
                printf("- - - Camera ortographic\n");
                printf("- - - ZFar %f\n", camera->zfar);
                printf("- - - ZNear %f\n", camera->znear);
                printf("- - - XMag %f\n", camera->xmag);
                printf("- - - YMag %f\n", camera->ymag);
                break;
            default:
                break;
        }
    }

    /*
     * Loading of the lights.
     */
    handle->lights = OS_MALLOC(sizeof(struct mdl_light) * data->lights_count);
    handle->lights_count = data->lights_count;
    os_memset(handle->lights, 0, sizeof(struct mdl_light) * data->lights_count);
    if (verbose) printf("- Lights count: %llu\n", (unsigned long long)data->lights_count);

    for (int32_t i = 0; i < data->lights_count; ++i) {
        struct cgltf_light *clight = data->lights + i;
        struct mdl_light *light = handle->lights + i;

        if (clight->name != 0) {
            light->name = OS_MALLOC(strlen(clight->name) + 1);
            os_memcpy(light->name, clight->name, strlen(clight->name) + 1);
            if (verbose) printf("- - Loading light: %s\n", light->name);
        } else {
            if (verbose) printf("- - Loading unnamed light\n");
            light->name = 0;
        }

        memcpy(light->color, clight->color, sizeof(float) * 3);
        light->intensity = clight->intensity;

        switch (clight->type) {
            case cgltf_light_type_invalid:
                break;
            case cgltf_light_type_directional:
                light->light_type = MDL_LIGHT_DIRECTIONAL;
                printf("- - - Light directional\n");
                break;
            case cgltf_light_type_point:
                light->light_type = MDL_LIGHT_POINT;
                printf("- - - Light point\n");
                break;
            case cgltf_light_type_spot:
                light->light_type = MDL_LIGHT_INVALID;
                printf("- - - Spot light is not supoorted\n");
                break;
            default:
                break;
        }

        printf("- - - Light color %f %f %f\n", light->color[0], light->color[1], light->color[2]);
        printf("- - - Light intensity %f\n", light->intensity);

    }

    /*
     * Loading of the meshes.
     */

    handle->meshes = OS_MALLOC(sizeof(struct mdl_mesh) * data->meshes_count);
    handle->meshes_count = data->meshes_count;
    os_memset(handle->meshes, 0, sizeof(struct mdl_mesh) * data->meshes_count);
    if (verbose) printf("- Meshes count: %llu\n", (unsigned long long)data->meshes_count);

    for (int32_t i = 0; i < data->meshes_count; ++i) {
        cgltf_mesh *cmesh = data->meshes + i;
        mdl_mesh *mesh = handle->meshes + i;

        if (cmesh->name != 0) {
            mesh->name = OS_MALLOC(strlen(cmesh->name) + 1);
            os_memcpy(mesh->name, cmesh->name, strlen(cmesh->name) + 1);

            if (verbose) printf("- - Loading mesh: %s\n", mesh->name);
        } else {
            if (verbose) printf("- - Loading unnamed mesh\n");
            mesh->name = 0;
        }
        mesh->primitives = OS_MALLOC(cmesh->primitives_count * sizeof(struct mdl_primitive));
        mesh->primitives_count = cmesh->primitives_count;

        if (verbose) printf("- - Primitives count: %llu\n", (unsigned long long)cmesh->primitives_count);

        for (int32_t j = 0; j < cmesh->primitives_count; ++j) {
            cgltf_primitive *cprimitive = cmesh->primitives + j;
            mdl_primitive *primitive = mesh->primitives + j;
            os_memset(primitive, 0, sizeof(mdl_primitive));

            if (verbose && cprimitive->indices != 0) {
                printf("- - - Primitive indices count: %llu\n", (unsigned long long)cprimitive->indices->count);
            } else if (verbose) {
                printf("- - - Primitive has no indices; generating sequential indices\n");
            }
            if (cprimitive->has_draco_mesh_compression) {
                printf("- - - Draco mesh compression is not implemented!\n");
                continue;
            }

            switch (cprimitive->type) {

                case cgltf_primitive_type_points:primitive->primitive_type = MDL_PRIMITIVE_TYPE_POINTS;
                    break;
                case cgltf_primitive_type_lines:primitive->primitive_type = MDL_PRIMITIVE_TYPE_LINES;
                    break;
                case cgltf_primitive_type_line_loop:primitive->primitive_type = MDL_PRIMITIVE_TYPE_LINE_LOOP;
                    break;
                case cgltf_primitive_type_line_strip:primitive->primitive_type = MDL_PRIMITIVE_TYPE_LINE_STRIP;
                    break;
                case cgltf_primitive_type_triangles:primitive->primitive_type = MDL_PRIMITIVE_TYPE_TRIANGLES;
                    break;
                case cgltf_primitive_type_triangle_strip:primitive->primitive_type = MDL_PRIMITIVE_TYPE_TRIANGLE_STRIP;
                    break;
                case cgltf_primitive_type_triangle_fan:primitive->primitive_type = MDL_PRIMITIVE_TYPE_TRIANGLE_FAN;
                    break;
                default:
                    primitive->primitive_type = MDL_PRIMITIVE_TYPE_TRIANGLES;
                    break;
            }

            if (cprimitive->indices != 0) {
                cgltf_accessor *indices_accessor = cprimitive->indices;
                if (indices_accessor->type != cgltf_type_scalar) {
                    printf("- - - Primitive indices must be scalar; generating sequential indices\n");
                } else {
                    primitive->indices_count = indices_accessor->count;
                    primitive->indices = OS_MALLOC(sizeof(uint32_t) * indices_accessor->count);

                    for (int32_t k = 0; k < indices_accessor->count; ++k) {
                        primitive->indices[k] = cgltf_accessor_read_index(indices_accessor, k);
                    }
                }
            }

            primitive->attributes_count = cprimitive->attributes_count;
            primitive->attributes = OS_MALLOC(sizeof(mdl_attribute) * cprimitive->attributes_count);
            primitive->vertex_stride = 0;
            primitive->vertices_count = 0;

            for (uint32_t k = 0; k < cprimitive->attributes_count; ++k) {
                cgltf_attribute *cattribute = cprimitive->attributes + k;
                mdl_attribute *attribute = primitive->attributes + k;
                switch (cattribute->type) {

                    case cgltf_attribute_type_invalid: printf("- - - - Invalid attribute encountered\n");
                        break;
                    case cgltf_attribute_type_position: attribute->type = MDL_VERTEX_ATTRIBUTE_POSITION;
                        break;
                    case cgltf_attribute_type_normal:attribute->type = MDL_VERTEX_ATTRIBUTE_NORMAL;
                        break;
                    case cgltf_attribute_type_tangent:attribute->type = MDL_VERTEX_ATTRIBUTE_TANGENT;
                        break;
                    case cgltf_attribute_type_texcoord:attribute->type = MDL_VERTEX_ATTRIBUTE_UV;
                        break;
                    case cgltf_attribute_type_color:attribute->type = MDL_VERTEX_ATTRIBUTE_COLOR;
                        break;
                    case cgltf_attribute_type_joints:attribute->type = MDL_VERTEX_ATTRIBUTE_JOINTS;
                        break;
                    case cgltf_attribute_type_weights:attribute->type = MDL_VERTEX_ATTRIBUTE_WEIGHTS;
                        break;
                    default:
                        attribute->type = MDL_VERTEX_ATTRIBUTE_INVALID;
                        break;

                }
                /* Track which attribute types are present on this primitive */
                primitive->attributes_flag |= (int32_t)attribute->type;
                //Todo: user defined attributes(they start with underscore in gltf specification) but clgtf seems to generate invalid attribute for them??
                if (cattribute->type == cgltf_attribute_type_invalid) continue;

                cgltf_accessor *attribute_accessor = cattribute->data;
                attribute->offset = primitive->vertex_stride;
                attribute->element_size = 4;
                attribute->count = 0;
                switch (attribute_accessor->type) {
                    case cgltf_type_invalid: printf("- - - - Invalid attribute type encountered\n");
                        break;
                    case cgltf_type_scalar: attribute->count = 1;
                        break;
                    case cgltf_type_vec2: attribute->count = 2;
                        break;
                    case cgltf_type_vec3: attribute->count = 3;
                        break;
                    case cgltf_type_vec4: attribute->count = 4;
                        break;
                    case cgltf_type_mat3: attribute->count = 9;
                        break;
                    case cgltf_type_mat4: attribute->count = 16;
                        break;
                    case cgltf_type_mat2: attribute->count = 4;
                        break;
                    default:
                        attribute->count = 0;
                        break;
                }

                primitive->vertex_stride += attribute->element_size * attribute->count;
                primitive->vertices_count = attribute_accessor->count;
                if (verbose)
                    printf("- - - - Attribute type: %i, count: %i, offset: %i, size: %i\n",
                             attribute->type, attribute->count, attribute->offset, attribute->element_size);
            }
            if (verbose)
                printf("- - - Vertices count: %i, stride: %i\n", primitive->vertices_count, primitive->vertex_stride);
            CORE_ASSERT(primitive->vertices_count != 0);
            primitive->vertices = OS_MALLOC(primitive->vertex_stride * primitive->vertices_count);
            for (uint32_t k = 0; k < cprimitive->attributes_count; ++k) {
                cgltf_attribute *cattribute = cprimitive->attributes + k;
                if (cattribute->type == cgltf_attribute_type_invalid) continue;
                mdl_attribute *attribute = primitive->attributes + k;
                cgltf_accessor *attribute_accessor = cattribute->data;
                for (uint32_t h = 0; h < attribute_accessor->count; ++h) {
                    float *ptr = (float *) (((char *) primitive->vertices + primitive->vertex_stride * h) +
                                            attribute->offset);
                    cgltf_accessor_read_float(attribute_accessor, h, ptr, attribute->count);
                }
            }

            if (primitive->indices == 0) {
                primitive->indices_count = primitive->vertices_count;
                primitive->indices = OS_MALLOC(sizeof(uint32_t) * primitive->indices_count);
                for (int32_t k = 0; k < primitive->indices_count; ++k) {
                    primitive->indices[k] = k;
                }
            }

            //associate material
            int32_t material_id = -1;
            if (cprimitive->material != 0) {
                for (int32_t k = 0; k < data->materials_count; ++k) {
                    cgltf_material *mat = data->materials + k;
                    if (mat == cprimitive->material) {
                        material_id = k;
                        break;
                    }
                }
            }
            primitive->material_id = material_id;

            //Todo: material mappings?
        }
    }


    /*
     * Loading of the materials.
     */
    handle->materials_count = data->materials_count > 0 ? data->materials_count : 1;
    handle->materials = OS_MALLOC(sizeof(struct mdl_material) * handle->materials_count);
    os_memset(handle->materials, 0, sizeof(struct mdl_material) * handle->materials_count);
    if (data->materials_count == 0) {
        mdl_material *mat = handle->materials;
        mat->valid = true;
        mat->color_texture_id = -1;
        mat->color_factor[0] = 0.8f;
        mat->color_factor[1] = 0.8f;
        mat->color_factor[2] = 0.8f;
        mat->color_factor[3] = 1.0f;
        mat->roughness_factor = 0.6f;
        mat->metallic_factor = 0.0f;
    }
    if (verbose) printf("- Materials count: %llu\n", (unsigned long long)data->materials_count);

    for (int32_t i = 0; i < data->materials_count; ++i) {
        cgltf_material *cmat = data->materials + i;
        mdl_material *mat = handle->materials + i;
        mat->valid = false;

        if (cmat->name != 0) {
            if (verbose) printf("- - Loading material: %s\n", cmat->name);

            mat->name = OS_MALLOC(strlen(cmat->name) + 1);
            os_memcpy(mat->name, cmat->name, strlen(cmat->name) + 1);
        } else {
            if (verbose) printf("- - Loading unnamed material \n");
            mat->name = 0;
        }

        if (cmat->has_pbr_metallic_roughness) {
            mat->valid = true;
            /* Resolve base color texture index */
            mat->color_texture_id = -1;
            if (cmat->pbr_metallic_roughness.base_color_texture.texture != NULL) {
                cgltf_image *cimg = cmat->pbr_metallic_roughness.base_color_texture.texture->image;
                if (cimg != NULL) {
                    for (int32_t k = 0; k < (int32_t)data->images_count; ++k) {
                        if (data->images + k == cimg) {
                            mat->color_texture_id = k;
                            break;
                        }
                    }
                }
            }
            mat->metallic_factor = cmat->pbr_metallic_roughness.metallic_factor;
            mat->roughness_factor = cmat->pbr_metallic_roughness.roughness_factor;
            os_memcpy(mat->color_factor, cmat->pbr_metallic_roughness.base_color_factor, sizeof(gl_vec4));
            if (verbose) {
                printf("- - - Material pbr roughness\n");
                printf("- - - - Roughness %f\n", mat->roughness_factor);
                printf("- - - - Color %f %f %f %f\n", mat->color_factor[0], mat->color_factor[1], mat->color_factor[2], mat->color_factor[3]);
            }

        } else {
            //unsupported material
            if (verbose) printf("- - - Unsupported material\n");
        };

    }

    /*
     * Loading of the textures (embedded images via buffer_view, or external URIs).
     * Must happen before cgltf_free() while buffer_view data is still valid.
     */
    handle->textures_count = (int32_t)data->images_count;
    if (data->images_count > 0) {
        handle->textures = OS_MALLOC(sizeof(mdl_texture) * data->images_count);
        os_memset(handle->textures, 0, sizeof(mdl_texture) * data->images_count);

        /* Extract base directory from path for external URI resolution */
        char dir_buf[512];
        os_memset(dir_buf, 0, sizeof(dir_buf));
        if (path != NULL) {
            const char *last_sep  = strrchr(path, '/');
            const char *last_bsep = strrchr(path, '\\');
            const char *last = (last_sep > last_bsep) ? last_sep : last_bsep;
            if (last) {
                size_t dir_len = (size_t)(last - path + 1);
                if (dir_len < sizeof(dir_buf))
                    os_memcpy(dir_buf, path, dir_len);
            }
        }

        for (int32_t i = 0; i < (int32_t)data->images_count; ++i) {
            cgltf_image *cimg = data->images + i;
            mdl_texture *tex  = handle->textures + i;
            tex->valid = false;

            int w = 0, h = 0, ch = 0;
            unsigned char *pixels = NULL;

            if (cimg->buffer_view != NULL) {
                /* Embedded image (base64-decoded or .glb buffer) */
                unsigned char *src = (unsigned char *)cimg->buffer_view->buffer->data
                                     + cimg->buffer_view->offset;
                pixels = stbi_load_from_memory(src, (int)cimg->buffer_view->size,
                                               &w, &h, &ch, 4);
            } else if (cimg->uri != NULL) {
                /* External file URI — resolve relative to the .gltf directory */
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "%s%s", dir_buf, cimg->uri);
                pixels = stbi_load(full_path, &w, &h, &ch, 4);
            }

            if (pixels) {
                tex->width    = w;
                tex->height   = h;
                tex->channels = 4;
                tex->size     = w * h * 4;
                tex->buffer   = OS_MALLOC(tex->size);
                os_memcpy(tex->buffer, pixels, tex->size);
                stbi_image_free(pixels);
                tex->valid = true;
                printf("- Loaded image %i: %ix%i\n", i, w, h);
            } else {
                fprintf(stderr, "- Failed to load image %i: %s\n", i, stbi_failure_reason());
            }
        }
    }

    OS_FREE(buffer);
    cgltf_free(data);
    return handle;
}

void mdl_unload(mdl_handle data) {

    for(int32_t i=0; i<data->nodes_count; ++i)
    {
        mdl_node* node = data->nodes + i;
        OS_FREE(node->name);
        if(node->children_id != 0)
            OS_FREE(node->children_id);
    }
    OS_FREE(data->nodes);

    for(int32_t i=0; i<data->meshes_count; ++i)
    {
        mdl_mesh * mesh = data->meshes + i;
        OS_FREE(mesh->name);

        for(int32_t j=0; j<mesh->primitives_count; ++j)
        {
            mdl_primitive * primitive = mesh->primitives + j;
            OS_FREE(primitive->attributes);
            OS_FREE(primitive->indices);
            OS_FREE(primitive->vertices);
        }

        OS_FREE(mesh->primitives);
    }
    OS_FREE(data->meshes);

    for(int32_t i=0; i<data->cameras_count; ++i) {
        struct mdl_camera *camera = data->cameras + i;
        OS_FREE(camera->name);

    }
    OS_FREE(data->cameras);


    for(int32_t i=0; i<data->lights_count; ++i) {
        struct mdl_light *light = data->lights + i;
        OS_FREE(light->name);

    }

    if(data->lights != 0)
        OS_FREE(data->lights);

    for(int32_t i=0; i<data->materials_count; ++i) {
        mdl_material *material = data->materials + i;
        OS_FREE(material->name);

    }
    OS_FREE(data->materials);

    for(int32_t i=0; i<data->textures_count; ++i) {
        mdl_texture *texture = data->textures + i;
        OS_FREE(texture->name);
        OS_FREE(texture->buffer);
    }

    if(data->textures != 0)
        OS_FREE(data->textures);

    if(data->name != 0)
        OS_FREE(data->name);

    OS_FREE(data);
}
