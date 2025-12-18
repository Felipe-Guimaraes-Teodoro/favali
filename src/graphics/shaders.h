#pragma once

#include <string>

#include "glad/glad.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"

extern const char* default_vs;
extern const char* default_fs;

extern const char* depth_vs;
extern const char* depth_fs;

extern const char* default_vs_instanced;
extern const char* default_fs_instanced;

extern const char* cube_map_vs;
extern const char* cube_map_fs;

extern const char* sun_vs;
extern const char* sun_fs;

unsigned int create_shader(const GLchar *const * src, GLenum type);

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

void shader_uniform_vec3(
    unsigned int program, 
    std::string name,
    glm::vec3 vec
);

void shader_uniform_float(
    unsigned int program, 
    std::string name,
    float f
);

unsigned int create_program(unsigned int vs, unsigned int fs);

#ifdef UBO_DEFINITION // cause i'm too lazy to separate this to its file

typedef struct {
    unsigned int UBO;
    size_t size;
} UniformBuffer;

template <typename T>
UniformBuffer create_ubo(const T *data, size_t size) {
    UniformBuffer ub = {0};
    ub.size = size;
    glGenBuffers(1, &ub.UBO);

    glBindBuffer(GL_UNIFORM_BUFFER, ub.UBO);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return ub;
}

/// Warning: UBO size will not change. 
/// Make sure the buffer has been initialized with proper size
template<typename T>
void update_ubo(UniformBuffer& buf, const T* data) {
    glBindBuffer(GL_UNIFORM_BUFFER, buf.UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, buf.size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void bind_ubo(
    const std::string& name, 
    GLuint binding, 
    UniformBuffer& buf, 
    unsigned int shader
);

#endif