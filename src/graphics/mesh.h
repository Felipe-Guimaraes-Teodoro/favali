#pragma once

#include <vector>
using std::vector;
#include "glad/glad.h"
#include "gtc/type_ptr.hpp"

struct Mesh {
    unsigned int VBO, VAO, EBO;

    vector<float> vertices;
    vector<unsigned int> indices;

    bool setup = false;

    Mesh() = default;

    // prevent copying
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // allow moving
    Mesh(Mesh&& other) noexcept {
        VBO = other.VBO;  other.VBO = 0;
        VAO = other.VAO;  other.VAO = 0;
        EBO = other.EBO;  other.EBO = 0;

        vertices = std::move(other.vertices);
        indices  = std::move(other.indices);
        setup = other.setup;
        other.setup = false;
    }

    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            destroy();

            VBO = other.VBO;  other.VBO = 0;
            VAO = other.VAO;  other.VAO = 0;
            EBO = other.EBO;  other.EBO = 0;

            vertices = std::move(other.vertices);
            indices  = std::move(other.indices);
            setup = other.setup;
            other.setup = false;
        }
        return *this;
    }

    void destroy();

    void draw(
        unsigned int program, 
        glm::mat4 model_mat, 
        glm::mat4 view_mat, 
        glm::mat4 proj_mat, 
        glm::vec4 col, 
        unsigned int texture,
        unsigned int draw_mode = GL_TRIANGLES
    ) const;

    ~Mesh();
};

void setup_mesh(Mesh& mesh);

Mesh create_mesh(vector<float>& vertices, vector<unsigned int>& indices);
Mesh empty_mesh();

struct InstancedMesh {
    unsigned int VBO, VAO, EBO;
    unsigned int IBO; // instance buffer object

    vector<float> vertices;
    vector<unsigned int> indices;
    vector<glm::mat4> instances;
    int last_instance_buffer_size;

    bool setup = false;

    InstancedMesh() = default;

    // prevent copying
    InstancedMesh(const InstancedMesh&) = delete;
    InstancedMesh& operator=(const InstancedMesh&) = delete;

    // allow moving
    InstancedMesh(InstancedMesh&& other) noexcept {
        VBO = other.VBO;    other.VBO = 0;
        VAO = other.VAO;    other.VAO = 0;
        EBO = other.EBO;    other.EBO = 0;
        IBO = other.IBO;    other.IBO = 0;

        vertices = std::move(other.vertices);
        indices  = std::move(other.indices);
        setup = other.setup;
        other.setup = false;
    }

    InstancedMesh& operator=(InstancedMesh&& other) noexcept {
        if (this != &other) {
            destroy();

            VBO = other.VBO;    other.VBO = 0;
            VAO = other.VAO;    other.VAO = 0;
            EBO = other.EBO;    other.EBO = 0;
            IBO = other.IBO;    other.IBO = 0;

            vertices = std::move(other.vertices);
            indices  = std::move(other.indices);
            setup = other.setup;
            other.setup = false;
        }
        return *this;
    }

    void destroy();

    void draw(
        unsigned int program, 
        glm::mat4 view_mat, 
        glm::mat4 proj_mat, 
        glm::vec4 col, 
        unsigned int texture,
        unsigned int draw_mode = GL_TRIANGLES
    ) const;

    ~InstancedMesh();
};

void setup_mesh_instanced(InstancedMesh& mesh);

InstancedMesh create_mesh_instanced(
    vector<float>& vertices, 
    vector<unsigned int>& indices,
    vector<glm::mat4>& instances
);

InstancedMesh mesh_to_instanced(
    Mesh &mesh
);

void update_instances(InstancedMesh& mesh);

void push_instance(InstancedMesh& mesh, const glm::mat4& instance);
void pop_instance(InstancedMesh& mesh);
void clear_instances(InstancedMesh& mesh);