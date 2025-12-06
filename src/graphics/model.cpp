#include "model.hpp"
#include "texture.h"

#include <vector>
using std::vector;

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

cgltf_accessor *get_accessor_from_primitive_type(const cgltf_primitive *prim, cgltf_attribute_type attr_type) {
    for (int attr = 0; attr < prim->attributes_count; attr++) {
        if (prim->attributes[attr].type == attr_type) {
            return prim->attributes[attr].data;
        }
    }

    return NULL;
}

void append_texture_from_material(Shape& dst, cgltf_material* material) {
    if (material) {
        cgltf_texture *tex = material->pbr_metallic_roughness.base_color_texture.texture;

        if (tex && tex->image) {
            cgltf_image *img = tex->image;

            if (img->uri) {
                printf("%s", img->uri);
                dst.texture = make_texture(img->uri);
            }
            else if (img->buffer_view) {printf("texture has buffer view embedded\n");}
            else {
                printf("mesh has NO TEXTURE WHATSOEVER\n");
            }
        }
    }
}

void append_vertex_data_from_primitive(cgltf_primitive* primitive, vector<float> &vertices) {
    cgltf_accessor *pos_accessor = get_accessor_from_primitive_type(
        primitive,  
        cgltf_attribute_type_position
    );
    
    cgltf_accessor *normal_accessor = get_accessor_from_primitive_type(
        primitive,  
        cgltf_attribute_type_normal
    );
    
    cgltf_accessor *uv_accessor = get_accessor_from_primitive_type(
        primitive,  
        cgltf_attribute_type_texcoord
    );

    if (pos_accessor && normal_accessor && uv_accessor) {
        float tmp_pos[3];
        float tmp_norm[3];
        float tmp_uv[2];

        int stride = 8;
        
        vertices.resize(pos_accessor->count * stride);

        for (int k = 0; k < pos_accessor->count; ++k) {
            cgltf_accessor_read_float(pos_accessor, k, tmp_pos, 3);
            cgltf_accessor_read_float(normal_accessor, k, tmp_norm, 3);
            cgltf_accessor_read_float(uv_accessor, k, tmp_uv, 2);
            
            int base = k * stride;

            vertices[base + 0] = tmp_pos[0];
            vertices[base + 1] = tmp_pos[1];
            vertices[base + 2] = tmp_pos[2];

            vertices[base + 3] = tmp_norm[0];
            vertices[base + 4] = tmp_norm[1];
            vertices[base + 5] = tmp_norm[2];

            vertices[base + 6] = tmp_uv[0];
            vertices[base + 7] = tmp_uv[1];
        }
    } // if accessors
}

void append_index_data_from_primitive(cgltf_primitive* primitive, vector<unsigned int> &indices) {
    if (primitive->indices) {
        cgltf_accessor *indices_accessor = primitive->indices;

        for (int index = 0; index < indices_accessor->count; index++) {
            indices.push_back(
                (unsigned int)cgltf_accessor_read_index(indices_accessor, index)
            );
        }
    }
}

cgltf_data *load_gltf(const char* path) {
    cgltf_options options = {};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);
    cgltf_result buf_result = cgltf_load_buffers(&options, data, path);

    if (result != cgltf_result_success) {
        printf("failed to load gltf file\n");

        return NULL;
    }

    if (buf_result != cgltf_result_success) {
        printf("failed to read gltf buffers\n");

        return NULL;
    }

    return data;
}

Shape create_shape_from_gltf(const char *path, int idx) {
    Shape shape = make_shape(Shapes::Empty);

    cgltf_data *data = load_gltf(path);

    if (!data)
        return shape;

    vector<unsigned int> indices = {}; 
    vector<float> vertices = {};

    for (int i = 0; i < data->meshes_count; i++) {
        cgltf_mesh *current_mesh = &data->meshes[i];

        for (int p = 0; p < current_mesh->primitives_count; p++) {
            cgltf_primitive *primitive = &current_mesh->primitives[p];

            append_index_data_from_primitive(primitive, indices);

            append_vertex_data_from_primitive(primitive, vertices);

            append_texture_from_material(shape, primitive->material);
        } // for primitives

    } // for meshes

    shape.mesh = create_mesh(vertices, indices);

    return shape;
}

Level *create_level_from_gltf(const char *path) {
    Level *level = create_level();

    cgltf_data *data = load_gltf(path);

    if (!data)
        return level;

    for (int i = 0; i < data->meshes_count; i++) {
        level->shapes.push_back(make_shape(Shapes::Empty));
    }

    for (int i = 0; i < data->meshes_count; i++) {
        cgltf_mesh *current_mesh = &data->meshes[i];

        for (int p = 0; p < current_mesh->primitives_count; p++) {
            cgltf_primitive *primitive = &current_mesh->primitives[p];

            append_index_data_from_primitive(primitive, level->shapes[i].mesh.indices);

            append_vertex_data_from_primitive(primitive, level->shapes[i].mesh.vertices);

            append_texture_from_material(level->shapes[i], primitive->material);
        } // for primitives

        setup_mesh(level->shapes[i].mesh);

    } // for meshes


    return level;
}