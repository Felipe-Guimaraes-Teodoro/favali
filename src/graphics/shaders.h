#pragma once

#include <string>

#include "glad/glad.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"

extern const char* default_vs;

extern const char* default_fs;

typedef struct {
    unsigned int id;
} Shader;

Shader create_shader(const GLchar *const * src, GLenum type);

void shader_uniform_mat4(
    unsigned int program, 
    std::string name,
    glm::mat4 mat
);

void shader_uniform_vec4(
    unsigned int program, 
    std::string name,
    glm::vec4 vec
);

unsigned int create_program(Shader& vs, Shader& fs);

#ifdef UBO_DEFINITION // cause i'm too lazy to separate this to its file

typedef struct {
    unsigned int UBO;
} UniformBuffer;

template<typename T>
UniformBuffer create_ubo(const T *data) {
    UniformBuffer ub = {0};
    glGenBuffers(1, &ub.UBO);

    glBindBuffer(GL_UNIFORM_BUFFER, ub.UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(T), data, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return ub;
}

template<typename T>
void update_ubo(UniformBuffer& buf, const T* data) {
    glBindBuffer(GL_UNIFORM_BUFFER, buf.UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void bind_ubo(
    const std::string& name, 
    GLuint binding, 
    UniformBuffer& buf, 
    Shader& shader
);

#endif