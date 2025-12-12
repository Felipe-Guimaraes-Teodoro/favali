#define UBO_DEFINITION
#include "shaders.h"

const char* default_vs = R"(
#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    TexCoord = aTexCoord;
}
)";

const char* default_fs = R"(
#version 420 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform mat4 view;
uniform vec4 color;
uniform sampler2D ourTexture;

const int MAX_LIGHTS = 20;

layout(std140, binding = 0) uniform Lights {
    vec3 positions[MAX_LIGHTS];
    vec3 colors[MAX_LIGHTS];
    int lightCount;
};

const vec3 ambient = vec3(0.1);

// uniform PointLight pointLights[16];

void main() {
    vec3 viewPos = -mat3(view) * view[3].xyz;

    vec3 normal = Normal;
    vec3 texColor = texture(ourTexture, TexCoord).rgb;

    vec3 result = vec3(0.0);

    for(int i = 0; i < lightCount; i++) {
        vec3 lightPos = positions[i];
        vec3 lightColor = colors[i];

        float distance = length(lightPos - FragPos);
        vec3 lightDir = normalize(lightPos - FragPos);

        float diff = max(dot(normal, lightDir), 0.1);

        float attenuation = 1.0 / max(pow(distance, 1.5), 0.01);

        result += color.rgb * lightColor * (diff + ambient) * texColor * (attenuation * 100);
    }

    FragColor = vec4(result, color.a);
}
)";

const char* default_vs_instanced = R"(
#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 aModel; // per instance

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * aModel * vec4(aPos, 1.0f);

    FragPos = vec3(aModel * vec4(aPos, 1.0));
    Normal = normalize(mat3(transpose(inverse(aModel))) * aNormal);
    TexCoord = aTexCoord;
}
)";

const char* default_fs_instanced = default_fs;

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

void bind_ubo(const std::string& name, GLuint binding, UniformBuffer& buf, Shader& shader) {
    GLuint index = glGetUniformBlockIndex(shader.id, name.c_str());
    glUniformBlockBinding(shader.id, index, binding);

    // coult also be glBindBufferRange
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, buf.UBO);
}