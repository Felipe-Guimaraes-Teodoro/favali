#include "mesh.h"

#include "shaders.h"

Mesh::~Mesh() {
    destroy();
}

void Mesh::destroy() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);

    VBO = VAO = EBO = 0; // bro i had no idea this could be done
    setup = false;
}

void Mesh::draw(
    unsigned int program, 
    glm::mat4 model_mat, 
    glm::mat4 view_mat, 
    glm::mat4 proj_mat, 
    glm::vec4 col, 
    unsigned int texture,
    unsigned int draw_mode // = GL_TRIANGLES by default
) const {
    if (!setup) {
        printf("WARNING: Attempting to draw deleted or incomplete mesh\n"); 
        return;
    }

    glUseProgram(program);
    
    shader_uniform_mat4(program, "model", model_mat);
    shader_uniform_mat4(program, "view", view_mat);
    shader_uniform_mat4(program, "projection", proj_mat);

    shader_uniform_vec4(program, "color", col);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElements(
        draw_mode,
        indices.size(), 
        GL_UNSIGNED_INT, 
        0
    );
}

void setup_mesh(Mesh& mesh) {
    mesh.setup = true;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    
    glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, 
                mesh.vertices.size() * sizeof(float), 
                mesh.vertices.data(), 
                GL_STATIC_DRAW
            );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                mesh.indices.size() * sizeof(unsigned int), 
                mesh.indices.data(), 
                GL_STATIC_DRAW
            );

        // POSITION
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // NORMAL
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // UV
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
}

Mesh create_mesh(vector<float>& vertices, vector<unsigned int>& indices) {
    Mesh mesh = {};
    mesh.vertices = vertices; // POSTIION, normal, uv
    mesh.indices = indices;
    
    setup_mesh(mesh);

    return mesh;
}

Mesh empty_mesh() {
    Mesh mesh = {};

    return mesh;
}


// instanced mesh

InstancedMesh::~InstancedMesh() {
    destroy();
}

void InstancedMesh::destroy() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (IBO) glDeleteBuffers(1, &IBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);

    VBO = VAO = EBO = IBO = 0;
    setup = false;
}

void InstancedMesh::draw(
    unsigned int program,
    glm::mat4 view_mat, 
    glm::mat4 proj_mat, 
    glm::vec4 col,
    unsigned int texture,
    unsigned int draw_mode // = GL_TRIANGLES by default
) const {
    if (!setup) {
        printf("WARNING: Attempting to draw deleted or incomplete mesh\n"); 
        return;
    }

    glUseProgram(program);
    
    shader_uniform_mat4(program, "view", view_mat);
    shader_uniform_mat4(program, "projection", proj_mat);

    shader_uniform_vec4(program, "color", col);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElementsInstanced(
        draw_mode,
        indices.size(), 
        GL_UNSIGNED_INT, 
        0,
        instances.size()
    );
}

void setup_mesh_instanced(InstancedMesh& mesh) {
    mesh.setup = true;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    glGenBuffers(1, &mesh.IBO);
    
    glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, 
                mesh.vertices.size() * sizeof(float), 
                mesh.vertices.data(), 
                GL_STATIC_DRAW
            );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                mesh.indices.size() * sizeof(unsigned int), 
                mesh.indices.data(), 
                GL_STATIC_DRAW
            );

            // POSITION
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // NORMAL
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // UV
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

        // instances
        glBindBuffer(GL_ARRAY_BUFFER, mesh.IBO);
            glBufferData(
                GL_ARRAY_BUFFER, 
                mesh.instances.size() * sizeof(float) * 16, 
                mesh.instances.data(),
                GL_STATIC_DRAW // GL_DYNAMIC_DRAW
            );

            // 4 * vector4
            for (int i = 0; i < 4; i++) {
                glVertexAttribPointer(
                    3 + i,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(float) * 16,
                    (void*)(i * sizeof(float) * 4)
                );
                glEnableVertexAttribArray(3 + i);
                glVertexAttribDivisor(3 + i, 1);
            }
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
}

InstancedMesh create_mesh_instanced(
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    std::vector<glm::mat4>& instances
) {
    InstancedMesh mesh;

    mesh.vertices = vertices;
    mesh.indices  = indices;
    mesh.instances = instances;
    mesh.last_instance_buffer_size = instances.size();

    setup_mesh_instanced(mesh);
    return mesh;
}

InstancedMesh mesh_to_instanced(Mesh &mesh) {
    InstancedMesh instanced_mesh;

    instanced_mesh.vertices = mesh.vertices;
    instanced_mesh.indices = mesh.indices;
    mesh.destroy();

    vector<glm::mat4> instances = {glm::identity<glm::mat4>()};

    instanced_mesh.instances = instances;
    instanced_mesh.last_instance_buffer_size = 1;

    setup_mesh_instanced(instanced_mesh);
    return instanced_mesh;
}

void update_instances(InstancedMesh& mesh) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.IBO);

    if (mesh.last_instance_buffer_size != mesh.instances.size()) {
        glBufferData(
            GL_ARRAY_BUFFER,
            mesh.instances.size() * sizeof(float) * 16,
            mesh.instances.data(),
            GL_STATIC_DRAW // GL_DYNAMIC_DRAW
        );
    } else {
        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            mesh.instances.size() * sizeof(float) * 16,
            mesh.instances.data()
        );
    }

    mesh.last_instance_buffer_size = mesh.instances.size();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void push_instance(InstancedMesh& mesh, const glm::mat4& instance) {
    mesh.instances.push_back(instance);
}

void pop_instance(InstancedMesh& mesh) {
    mesh.instances.pop_back();
}

/// doesnt actually clear, just sets the instances to a single identity matrix
void clear_instances(InstancedMesh& mesh) {
    mesh.instances = {glm::identity<glm::mat4>()};
}

// Cube Map business

CubeMapMesh create_cube_map() {
    std::vector<float> vertices = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        3, 4, 5,

        6, 7, 8,
        9, 10, 11,

        12, 13, 14,
        15, 16, 17,

        18, 19, 20,
        21, 22, 23,

        24, 25, 26,
        27, 28, 29,

        30, 31, 32,
        33, 34, 35
    };

    CubeMapMesh mesh;

    mesh.vertices = vertices;
    mesh.indices  = indices;

    setup_cube_map_mesh(mesh);
    return mesh;
}

void setup_cube_map_mesh(CubeMapMesh& mesh) {
    mesh.setup = true;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    
    glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, 
                mesh.vertices.size() * sizeof(float), 
                mesh.vertices.data(), 
                GL_STATIC_DRAW
            );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                mesh.indices.size() * sizeof(unsigned int), 
                mesh.indices.data(), 
                GL_STATIC_DRAW
            );

        // POSITION
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
}

CubeMapMesh::~CubeMapMesh() {
    destroy();
}

void CubeMapMesh::destroy() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);

    VBO = VAO = EBO = 0;
    setup = false;
}

void CubeMapMesh::draw(
    unsigned int program, 
    glm::mat4 model_mat, 
    glm::mat4 view_mat, 
    glm::mat4 proj_mat
) const {
    if (!setup) {
        printf("WARNING: Attempting to draw deleted or incomplete cube map mesh\n"); 
        return;
    }

    glUseProgram(program);
    
    shader_uniform_mat4(program, "model", model_mat);
    shader_uniform_mat4(program, "view", view_mat);
    shader_uniform_mat4(program, "projection", proj_mat);

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glBindVertexArray(VAO);
    glDrawElements(
        GL_TRIANGLES,
        indices.size(), 
        GL_UNSIGNED_INT, 
        0
    );
}
