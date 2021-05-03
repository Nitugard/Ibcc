/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Model.h"

#include <Os/Allocator.h>
#include <Os/Log.h>
#include <Os/File.h>
#include <Math/GlMath.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>
#include <assert.h>
#include <stbi/stbi.h>


/*
 * Known limitations:
 * 1. Only 1 uv channel per primitive supported
 * 2. Only 8 textures currently supported
 * 3. Only pbr metallic roughness material supported
 * 4. No skinning
 * 5. ?
 */


API mdl_handle mdl_load(mdl_desc* desc) {
    bool verbose = true;

    size_t size;
    void *buffer = file_mmap(desc->path, &size);
    if (buffer == 0) {
        LOG_ERROR("File could not be read: %s\n", desc->path);
        return 0;
    }

    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, buffer, size, &data);
    if (result != cgltf_result_success) {
        return 0;
    }

    cgltf_load_buffers(&options, data, desc->path);

    mdl_handle handle = OS_MALLOC(sizeof(mdl_data));
    os_memset(handle, 0, sizeof(mdl_data));

    /*
     * Loading of the scenes. Todo.
     */

    /*
     * Loading of the nodes.
     */

    if (verbose) LOG_INFO("Nodes count: %i\n", data->nodes_count);

    handle->nodes_count = data->nodes_count;
    handle->nodes = OS_MALLOC(sizeof(struct mdl_node) * data->nodes_count);
    os_memset(handle->nodes, 0, sizeof(struct mdl_node) * data->nodes_count);

    for (int32_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node *cnode = data->nodes + i;
        mdl_node *node = handle->nodes + i;
        node->mesh_index = node->light_index = node->camera_index = -1;
        node->name = OS_MALLOC(strlen(cnode->name) + 1);
        os_memcpy(node->name, cnode->name, strlen(cnode->name) + 1);
        if (verbose) LOG_INFO("Loading node: %s\n", node->name);


        if (cnode->has_matrix) {
            gl_mat local_mat = gl_mat_transpose(gl_mat_new_array(cnode->matrix));
            os_memcpy(node->tr_local, local_mat.data, sizeof(gl_mat));
        } else {

            gl_mat translate = GL_MAT_IDENTITY;
            gl_mat rotate = GL_MAT_IDENTITY;
            gl_mat scale = GL_MAT_IDENTITY;

            if (cnode->has_translation) {
                translate = gl_mat_translate(gl_vec3_new_arr(cnode->translation));
            }
            if (cnode->has_rotation) {
                rotate = gl_mat_from_quaternion(gl_vec4_new_arr(cnode->rotation));
            }
            if (cnode->has_scale) {
                scale = gl_mat_scale(gl_vec3_new_arr(cnode->scale));
            }
            gl_mat trs = gl_mat_mul(translate, gl_mat_mul(rotate, scale));
            os_memcpy(node->tr_local, trs.data, sizeof(gl_mat));
        }

        //load children
        if (verbose) LOG_INFO("Children count: %i\n", cnode->children_count);
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
                if (child_index == -1) { LOG_ERROR("Child node index could not be found!"); }
                else {
                    node->children_id[j] = child_index;
                    if (verbose) LOG_INFO("\tAdded child: %s, index: %i\n", child_cnode->name, child_index);
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

        if (cnode->mesh) {
            int32_t mesh_index = -1;
            for (int32_t j = 0; j < data->meshes_count; ++j) {
                if (data->meshes + j == cnode->mesh) {
                    mesh_index = j;
                    break;
                }
            }
            if (mesh_index == -1) { LOG_ERROR("Node mesh index could not be found!"); }
            else {
                node->mesh_index = mesh_index;
                if (verbose) LOG_INFO("Node mesh: %s, index: %i\n", cnode->mesh->name, mesh_index);
            }
        }
    }


    /*
     * Loading of the meshes.
     */

    handle->meshes = OS_MALLOC(sizeof(struct mdl_mesh) * data->meshes_count);
    handle->meshes_count = data->meshes_count;
    os_memset(handle->meshes, 0, sizeof(struct mdl_mesh) * data->meshes_count);
    if (verbose) LOG_INFO("Meshes count: %i\n", data->meshes_count);

    for (int32_t i = 0; i < data->meshes_count; ++i) {
        cgltf_mesh *cmesh = data->meshes + i;
        mdl_mesh *mesh = handle->meshes + i;

        if (cmesh->name != 0) {
            mesh->name = OS_MALLOC(strlen(cmesh->name) + 1);
            os_memcpy(mesh->name, cmesh->name, strlen(cmesh->name) + 1);

            if (verbose) LOG_INFO("Loading mesh: %s\n", mesh->name);
        } else {
            if (verbose) LOG_INFO("Loading unnamed mesh\n");
            mesh->name = 0;
        }
        mesh->primitives = OS_MALLOC(cmesh->primitives_count * sizeof(struct mdl_primitive));
        mesh->primitives_count = cmesh->primitives_count;

        if (verbose) LOG_INFO("Primitives count: %i\n", cmesh->primitives_count);

        for (int32_t j = 0; j < cmesh->primitives_count; ++j) {
            cgltf_primitive *cprimitive = cmesh->primitives + j;
            mdl_primitive *primitive = mesh->primitives + j;
            os_memset(primitive, 0, sizeof(mdl_primitive));

            if (verbose) LOG_INFO("Primitive indices count: %i\n", cprimitive->indices->count);
            if (cprimitive->has_draco_mesh_compression) {
                LOG_ERROR("Draco mesh compression is not implemented!\n");
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
            }

            cgltf_accessor *indices_accessor = cprimitive->indices;
            if (indices_accessor->type != cgltf_type_scalar) {
                LOG_ERROR("Primitive indices must be scalar");
                continue;
            }

            primitive->indices_count = indices_accessor->count;
            primitive->indices = OS_MALLOC(sizeof(uint32_t) * indices_accessor->count);

            for (int32_t k = 0; k < indices_accessor->count; ++k) {
                primitive->indices[k] = cgltf_accessor_read_index(indices_accessor, k);
            }

            primitive->attributes_count = cprimitive->attributes_count;
            primitive->attributes = OS_MALLOC(sizeof(mdl_attribute) * cprimitive->attributes_count);
            primitive->vertex_stride = 0;
            primitive->vertices_count = 0;

            for (uint32_t k = 0; k < cprimitive->attributes_count; ++k) {
                cgltf_attribute *cattribute = cprimitive->attributes + k;
                mdl_attribute *attribute = primitive->attributes + k;
                switch (cattribute->type) {

                    case cgltf_attribute_type_invalid: LOG_ERROR("Invalid attribute encountered\n");
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

                }
                //Todo: user defined attributes(they start with underscore in gltf specification) but clgtf seems to generate invalid attribute for them??
                if (cattribute->type == cgltf_attribute_type_invalid) continue;

                cgltf_accessor *attribute_accessor = cattribute->data;
                attribute->offset = primitive->vertex_stride;
                attribute->element_size = 4;
                attribute->count = 0;
                switch (attribute_accessor->type) {
                    case cgltf_type_invalid: LOG_ERROR("Invalid attribute type encountered\n");
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
                }

                primitive->vertex_stride += attribute->element_size * attribute->count;
                primitive->vertices_count = attribute_accessor->count;
                if (verbose)
                    LOG_INFO("Attribute type: %i, count: %i, offset: %i, size: %i\n",
                             attribute->type, attribute->count, attribute->offset, attribute->element_size);
            }
            if (verbose)
                LOG_INFO("Vertices count: %i, stride: %i\n", primitive->vertices_count, primitive->vertex_stride);
            assert(primitive->vertices_count != 0);
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

            //associate material
            uint32_t material_id = -1;
            if (cprimitive->material != 0) {
                for (uint32_t k = 0; k < data->materials_count; ++k) {
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
    handle->materials = OS_MALLOC(sizeof(struct mdl_material) * data->materials_count);
    handle->materials_count = data->materials_count;
    os_memset(handle->materials, 0, sizeof(struct mdl_material) * data->materials_count);
    if (verbose) LOG_INFO("Materials count: %i\n", data->materials_count);

    for (int32_t i = 0; i < data->materials_count; ++i) {
        cgltf_material *cmat = data->materials + i;
        mdl_material *mat = handle->materials + i;
        mat->valid = false;

        if (cmat->name != 0) {
            if (verbose) LOG_INFO("Loading material: %s\n", cmat->name);

            mat->name = OS_MALLOC(strlen(cmat->name) + 1);
            os_memcpy(mat->name, cmat->name, strlen(cmat->name) + 1);
        } else {
            if (verbose) LOG_INFO("Loading unnamed material \n");
            mat->name = 0;
        }

        if (cmat->has_pbr_metallic_roughness) {
            mat->valid = true;
            mat->unlit = cmat->unlit;
            mat->color_texture_id = -1;

            if (verbose) LOG_INFO("Material pbr roughness\n");
            for (int32_t k = 0; k < data->textures_count; ++k) {
                cgltf_texture *tex = data->textures + k;
                if (tex == cmat->pbr_metallic_roughness.base_color_texture.texture) {
                    mat->color_texture_id = k;
                    break;
                }
            }
            os_memcpy(mat->color_factor, cmat->pbr_metallic_roughness.base_color_factor, sizeof(gl_vec4));
            mat->alpha_cutoff = cmat->alpha_cutoff;
        } else {
            //unsupported material
            if (verbose) LOG_INFO("Unsupported material\n");
        };

    }



    /*
     * Loading of the textures.
     */
    if(desc->load_textures) {
        handle->textures_count = data->textures_count;
        handle->textures = OS_MALLOC(sizeof(struct mdl_texture) * data->textures_count);
        os_memset(handle->textures, 0, sizeof(struct mdl_texture) * data->textures_count);

        if (verbose) LOG_INFO("Textures count %i\n", handle->textures_count);

        for (int32_t i = 0; i < data->textures_count; ++i) {

            cgltf_texture *ctex = data->textures + i;
            mdl_texture *tex = handle->textures + i;
            tex->valid = false;
            cgltf_buffer_view *tex_buffer_view = ctex->image->buffer_view;
            if (ctex->name != 0) {
                if (verbose) LOG_INFO("Loading texture: %s\n", ctex->name);

                tex->name = OS_MALLOC(strlen(ctex->name) + 1);
                os_memcpy(tex->name, ctex->name, strlen(ctex->name) + 1);
            } else {
                if (verbose) LOG_INFO("Loading unnamed texture\n");
                tex->name = 0;
            }

            assert(tex_buffer_view->buffer->data != 0);

            int32_t width, height, channels;

            unsigned char *ptr = (unsigned char *) tex_buffer_view->buffer->data + tex_buffer_view->offset;
            if (stbi_is_16_bit_from_memory(ptr, tex_buffer_view->size)) {
                LOG_INFO("Texture is 16 bit, currently not supported!\n");
                continue;
            }

            unsigned char *loaded_data = stbi_load_from_memory(ptr, tex_buffer_view->size, &width, &height, &channels,
                                                               0);
            if (loaded_data == 0) {
                LOG_ERROR("Failed to load texture!\n");
                continue;
            }
            tex->valid = true;
            tex->width = width;
            tex->height = height;
            tex->channels = channels;
            tex->size = width * height * channels;
            int32_t bsize = width * height * channels * sizeof(unsigned char);
            tex->buffer = OS_MALLOC(bsize);
            os_memcpy(tex->buffer, loaded_data, bsize);
            stbi_image_free(loaded_data);
        }
    }
    else{
        handle->textures_count = 0;
        handle->textures = 0;
    }

    cgltf_free(data);
    return handle;
}

void mdl_unload(mdl_handle data) {

    for(int32_t i=0; i<data->nodes_count; ++i)
    {
        mdl_node* node = data->nodes + i;
        OS_FREE(node->name);
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

    OS_FREE(data->textures);
    OS_FREE(data->name);

    OS_FREE(data);
}
