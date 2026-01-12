#include "model.hpp"
#include "texture.h"

#include <stdlib.h>
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

unsigned int create_texture_from_material(cgltf_material* material) {
    if (!material) {
        return create_default_texture();
    }
    cgltf_texture *tex = material->pbr_metallic_roughness.base_color_texture.texture;

    if (!tex || !tex->image) {
        return create_default_texture();
    }

    cgltf_image *img = tex->image;

    if (img->uri) {
        char path[512];
        snprintf(path, sizeof(path), "assets/%s", img->uri);
        return make_texture(path);
    }

    if (img->buffer_view) {
        int size = img->buffer_view->size;
        // printf("texture has buffer view with size %u embedded\n");
    
        const uint8_t *tex_data = cgltf_buffer_view_data(img->buffer_view);

        return make_texture_from_memory(tex_data, size);
    }

    return create_default_texture();
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
    Shape shape = make_shape(Shapes::Triangle); // so if something goes wrong render placeholder shape

    cgltf_data *data = load_gltf(path);

    if (!data)
        return shape;

    vector<unsigned int> indices = {}; 
    vector<float> vertices = {};
    int texture = 0;

    for (int i = 0; i < data->meshes_count; i++) {
        cgltf_mesh *current_mesh = &data->meshes[i];

        for (int p = 0; p < current_mesh->primitives_count; p++) {
            cgltf_primitive *primitive = &current_mesh->primitives[p];

            append_index_data_from_primitive(primitive, indices);
            append_vertex_data_from_primitive(primitive, vertices);
            texture = create_texture_from_material(primitive->material);
        } // for primitives

    } // for meshes

    shape.mesh = create_mesh(vertices, indices);
    shape.texture = texture;

    return shape;
}

Level *create_level_from_gltf(const char *path) {
    Level *level = create_level();
    cgltf_data *data = load_gltf(path);

    if (!data)
        return level;

    int primitive_count = 0;
    for (int i = 0; i < data->meshes_count; i++) {
        primitive_count += data->meshes[i].primitives_count;
    }

    level->shapes.reserve(primitive_count);
    // printf("%u shapes\n", primitive_count);
    unsigned int texture = 0;
    int light_count = 0;

    std::vector<unsigned int> indices;
    std::vector<float> vertices;

    for (int i = 0; i < data->nodes_count; i++) {
        cgltf_node* node = &data->nodes[i];
        // printf("loading node %s\n", node->name);

        // load lights
        if (node->light) {
            // printf("loading light\n");
            cgltf_light *light = node->light;

            vec3 translation = vec3(0);
            if (node->has_translation) {
                translation = {node->translation[0], node->translation[1], node->translation[2]};
            }

            level->lights.position[light_count].v = translation;
            level->lights.color[light_count].v = {light->color[0], light->color[1], light->color[2]};
            light_count++;
            level->lights.count = light_count;
        }

        // load meshes
        if (!node->mesh) continue;

        cgltf_mesh* mesh = node->mesh;

        glm::vec3 node_pos(node->translation[0], node->translation[1], node->translation[2]);
        glm::quat node_rot(node->rotation[3], glm::vec3(node->rotation[0], node->rotation[1], node->rotation[2]));
        glm::vec3 node_scale(node->scale[0], node->scale[1], node->scale[2]);

        for (int p = 0; p < mesh->primitives_count; p++){
            cgltf_primitive* prim = &mesh->primitives[p];

            vertices.clear();
            indices.clear();

            append_index_data_from_primitive(prim, indices);
            append_vertex_data_from_primitive(prim, vertices);
            texture = create_texture_from_material(prim->material);

            // this part is extremely slow
            Mesh mesh = create_mesh(vertices, indices);
            // todo: merge all meshes with the same material
            // onto one big shared mesh. it would reduce draw calls too

            Transform transform = Transform::empty();
            transform.position = node_pos;
            transform.rotation  = node_rot;
            transform.scale = node_scale;

            level->shapes.push_back(
                shape_to_static(
                    Shape(
                        std::move(mesh),
                        transform,
                        glm::vec4(1.0f),
                        texture
                    )
                )
            );
        }

        // append_instance_data_from_node(): 
        if (node->has_mesh_gpu_instancing) {
            printf("Node has gpu instancing\n");
            for (int attr = 0; attr < node->mesh_gpu_instancing.attributes_count; attr++) {
                printf("%s\n", node->mesh_gpu_instancing.attributes->name);
            }
        }
    }

    return level;
}


Model *create_model_from_gltf(const char *path) {
    Model *level = create_model();
    cgltf_data *data = load_gltf(path);

    if (!data)
        return level;

    int primitive_count = 0;
    for (int i = 0; i < data->meshes_count; i++) {
        primitive_count += data->meshes[i].primitives_count;
    }

    level->shapes.reserve(primitive_count);
    printf("%u shapes\n", primitive_count);
    unsigned int texture = 0;
    int light_count = 0;

    std::vector<unsigned int> indices;
    std::vector<float> vertices;

    for (int i = 0; i < data->nodes_count; i++) {
        cgltf_node* node = &data->nodes[i];
        printf("loading node %s\n", node->name);

        // load lights
        if (node->light) {
            printf("loading light\n");
            cgltf_light *light = node->light;

            vec3 translation = vec3(0);
            if (node->has_translation) {
                translation = {node->translation[0], node->translation[1], node->translation[2]};
            }

            level->lights.position[light_count].v = translation;
            level->lights.color[light_count].v = {light->color[0], light->color[1], light->color[2]};
            light_count++;
            level->lights.count = light_count;
        }

        // load meshes
        if (!node->mesh) continue;

        cgltf_mesh* mesh = node->mesh;

        glm::vec3 node_pos(node->translation[0], node->translation[1], node->translation[2]);
        glm::quat node_rot(node->rotation[3], glm::vec3(node->rotation[0], node->rotation[1], node->rotation[2]));
        glm::vec3 node_scale(node->scale[0], node->scale[1], node->scale[2]);

        for (int p = 0; p < mesh->primitives_count; p++){
            cgltf_primitive* prim = &mesh->primitives[p];

            vertices.clear();
            indices.clear();

            append_index_data_from_primitive(prim, indices);
            append_vertex_data_from_primitive(prim, vertices);
            texture = create_texture_from_material(prim->material);

            // this part is extremely slow
            Mesh mesh = create_mesh(vertices, indices);
            // todo: merge all meshes with the same material
            // onto one big shared mesh. it would reduce draw calls too

            Transform transform = Transform::empty();
            transform.position = node_pos;
            transform.rotation  = node_rot;
            transform.scale = node_scale;

            level->shapes.push_back(
                shape_to_static(
                    Shape(
                        std::move(mesh),
                        transform,
                        glm::vec4(1.0f),
                        texture
                    )
                )
            );
        }

        // append_instance_data_from_node(): 
        if (node->has_mesh_gpu_instancing) {
            // printf("Node has gpu instancing\n");
            for (int attr = 0; attr < node->mesh_gpu_instancing.attributes_count; attr++) {
                // printf("%s\n", node->mesh_gpu_instancing.attributes->name);
            }
        }
    }

    return level;
}
