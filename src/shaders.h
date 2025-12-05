#pragma once

#include <string>

#include "glad/glad.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"

const char* default_vs = R"(
#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; 
}
)";

const char* default_fs = R"(
#version 420 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform mat4 view;
uniform vec4 color;

layout (std140, binding = 0) uniform Light {
    vec3 position;
    vec3 color;
} light;

// uniform PointLight pointLights[16];

void main() {
    vec3 viewPos = -transpose(mat3(view)) * view[3].xyz;
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(Normal, lightDir), 0.1);

    vec3 result = color.rgb * light.color * diff;
    FragColor = vec4(result, color);
}
)";

typedef struct {
    unsigned int id;
} Shader;

Shader create_shader(const GLchar *const * src, GLenum type) {
    Shader sha = {};

    sha.id = glCreateShader(type);
    glShaderSource(sha.id, 1, src, NULL);
    glCompileShader(sha.id);

    int success;
    char infoLog[512];
    glGetShaderiv(sha.id, GL_COMPILE_STATUS, &success);

    if(!success) {
        glGetShaderInfoLog(sha.id, 512, NULL, infoLog);
        printf("%s\n", infoLog);
    }

    return sha;
}

void shader_uniform_mat4(
    unsigned int program, 
    std::string name,
    glm::mat4 mat
) {
    unsigned int location = glGetUniformLocation(program, name.data());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void shader_uniform_vec4(
    unsigned int program, 
    std::string name,
    glm::vec4 vec
) {
    unsigned int location = glGetUniformLocation(program, name.data());
    glUniform4fv(location, 1, glm::value_ptr(vec));
}

unsigned int create_program(Shader& vs, Shader& fs) {
    unsigned int program;
    program = glCreateProgram();
    glAttachShader(program, vs.id);
    glAttachShader(program, fs.id);
    glLinkProgram(program);

    int success;
    char infoLog[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        printf("%s\n", infoLog);
    }

    return program;
}

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

void bind_ubo(const std::string& name, GLuint binding, UniformBuffer& buf, Shader& shader) {
    GLuint index = glGetUniformBlockIndex(shader.id, name.c_str());
    glUniformBlockBinding(shader.id, index, binding);

    // coult also be glBindBufferRange
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, buf.UBO);
}