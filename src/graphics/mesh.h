#pragma once

#include <vector>
using std::vector;
#include "glad/glad.h"
#include "gtc/type_ptr.hpp"

struct Mesh {
    unsigned int VBO, VAO, EBO;

    vector<float> vertices;
    vector<unsigned int> indices;

    bool setup;

    void draw(
        unsigned int program, 
        glm::mat4 model_mat, 
        glm::mat4 view_mat, 
        glm::mat4 proj_mat, 
        glm::vec4 col, 
        unsigned int texture
    ) const;
};

void setup_mesh(Mesh& mesh);

Mesh create_mesh(vector<float>& vertices, vector<unsigned int>& indices);