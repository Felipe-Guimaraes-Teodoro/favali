#pragma once

#include "glm.hpp"
#include "glad/glad.h"
#include <vector>

#define SPOTLIGHT_SHADOW_SIZE 4096

class Level;

struct SpotLight{
    glm::vec3 color;
    glm::vec3 dir;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 lightSpaceMatrix;

    unsigned int depthProgram;
    unsigned int depthFBO;
    unsigned int depthMap;

    float fov;

    void updateLight(glm::vec3 lightPos);

    void renderShadow(unsigned int depthProgram, const std::vector<Level*>& levels, int screenW, int screenH, int shadowSize = SPOTLIGHT_SHADOW_SIZE);
};

SpotLight createSpotLight(int shadowSize = SPOTLIGHT_SHADOW_SIZE);

struct SpotLightsData{
    glm::vec4 positions[10];
    glm::vec4 directions[10];
    glm::vec4 colors[10];
    float cuttofs[10];
    int count;
};

