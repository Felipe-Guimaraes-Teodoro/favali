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
out vec4 FragPosSunSpace;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    TexCoord = aTexCoord;
    FragPosSunSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}
)";

const char* default_fs = R"(
#version 420 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosSunSpace;

out vec4 FragColor;

uniform bool useSun;
uniform mat4 view;
uniform vec4 color;
uniform vec3 sunDir;
uniform sampler2D ourTexture;
uniform sampler2D shadowMap;

const int MAX_LIGHTS = 20;

layout(std140, binding = 0) uniform Lights {
    vec3 positions[MAX_LIGHTS];
    vec3 colors[MAX_LIGHTS];
    int lightCount;
};

const vec3 ambient = vec3(0.3);

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.001 * (1.0 - dot(normalize(Normal), -sunDir)), 0.0005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
projCoords.y < 0.0 || projCoords.y > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main() {
    vec4 texColor = texture(ourTexture, TexCoord);

    vec3 normal = Normal;

    vec3 result = vec3(0.0);

    for(int i = 0; i < lightCount; i++) {
        vec3 lightPos = positions[i];
        vec3 lightColor = colors[i];

        float distance = length(lightPos - FragPos);
        vec3 lightDir = normalize(lightPos - FragPos);

        float diff = max(dot(normal, lightDir), 0.1);

        float attenuation = 1.0 / max(pow(distance, 1.5), 0.01);

        result += color.rgb * lightColor * (diff + ambient) * (attenuation * 100);
    }
    
    vec3 lighting = result;
    
    float shadow = 0.0;
    if (useSun) {
        shadow = ShadowCalculation(FragPosSunSpace);
    }
    
    lighting = mix(lighting, lighting * ambient.r, shadow);
    FragColor = vec4(lighting, color.a) * texColor;
}
)";

const char* depth_vs = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";

const char* depth_fs = R"(
#version 330 core

void main(){}
)";
/*
#version 330 core

// Ouput data
layout(location = 0) out float fragmentdepth;

void main(){
    // Not really needed, OpenGL does it anyway
    fragmentdepth = gl_FragCoord.z;
}
*/

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

const char* cube_map_vs = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    vec4 pos = projection * view * vec4 (aPos, 1.0);
    gl_Position = pos.xyww;
}  
)";

const char* cube_map_fs = R"(
#version 330 core
out vec4 FragColor;

in vec3 TexCoords; // direção do fragmento na skybox

uniform samplerCube daySky;
uniform samplerCube nightSky;
uniform sampler3D cloudNoise3D;

uniform vec3 sunDir;
uniform vec3 sunColor;
uniform float sunIntensity;

void main()
{
    vec3 dir = normalize(TexCoords);

    vec3 dayColor   = texture(daySky, dir).rgb;
    vec3 nightColor = texture(nightSky, dir).rgb;
    float dayFactor = clamp(sunDir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 sky = mix(nightColor, dayColor, dayFactor);

    float sunsetFade = smoothstep(-0.1, 0.05, sunDir.y);
    float horizonFactor = clamp((dir.y + 0.1) * 0.7, 0.0, 1.0);
    vec3 sunsetColor = vec3(1.0, 0.4, 0.2);
    sky = mix(sky, sunsetColor, (1.0 - dayFactor) * (1.0 - horizonFactor) * sunsetFade);

    float sunAmount = max(dot(dir, normalize(sunDir)), 0.0);
    float glowPower = mix(32.0, 8.0, 1.0 - dayFactor);
    float sunGlow = pow(sunAmount, glowPower) * sunIntensity * dayFactor;
    vec3 sunGlowColor = mix(sunColor, sunsetColor, 1.0 - dayFactor);
    sky += sunGlowColor * sunGlow;

    FragColor = vec4(sky, 1.0);
}

)";

const char* sun_vs = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    uv = aTexCoord;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}  
)";

const char* sun_fs = R"(
#version 330 core
in vec2 uv;
out vec4 FragColor;

uniform vec3 sunColor;
uniform float sunIntensity;

void main() {
    float d = length(uv - vec2(0.5));

    float alpha = 1.0 - smoothstep(0.3, 0.5, d);

    vec3 color = sunColor * sunIntensity;

    FragColor = vec4(color, alpha);
}
)";

unsigned int create_shader(const GLchar *const * src, GLenum type) {
    unsigned int sha = {};

    sha = glCreateShader(type);
    glShaderSource(sha, 1, src, NULL);
    glCompileShader(sha);

    GLint ok;
    glGetShaderiv(sha, GL_COMPILE_STATUS, &ok);
    if (!ok){
        char log[1024];
        glGetShaderInfoLog(sha, 1024, NULL, log);
        printf("compile error at: %s\n", log);
        glDeleteShader(sha);
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

void shader_uniform_vec3(
    unsigned int program, 
    std::string name,
    glm::vec3 vec
) {
    unsigned int location = glGetUniformLocation(program, name.data());
    glUniform3fv(location, 1, glm::value_ptr(vec));
}

void shader_uniform_float(
    unsigned int program, 
    std::string name,
    float f
) {
    unsigned int location = glGetUniformLocation(program, name.data());
    glUniform1f(location, f);
}

unsigned int create_program(unsigned int vs, unsigned int fs) {
    unsigned int program;
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    GLint ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok){
        char log[1024];
        glGetProgramInfoLog(program, 1024, NULL, log);
        printf("link error at: %s\n", log);
    }

    return program;
}

void bind_ubo(const std::string& name, GLuint binding, UniformBuffer& buf, unsigned int shader) {
    GLuint index = glGetUniformBlockIndex(shader, name.c_str());
    glUniformBlockBinding(shader, index, binding);

    // coult also be glBindBufferRange
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, buf.UBO);
}