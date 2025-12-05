#include "mesh.h"

#include "shaders.h"

void Mesh::draw(
    unsigned int program, 
    glm::mat4 model_mat, 
    glm::mat4 view_mat, 
    glm::mat4 proj_mat, 
    glm::vec4 col, 
    unsigned int texture
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
        GL_TRIANGLES, 
        indices.size(), 
        GL_UNSIGNED_INT, 
        0
    );
}


void setup_mesh(Mesh& mesh) {
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
    mesh.setup = true;

    return mesh;
}