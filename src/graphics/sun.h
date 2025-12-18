#pragma once

#include "glm.hpp"
#include "glad/glad.h"
#include <vector>

#define SHADOW_SIZE 2048

class Level;

struct Sun{
    glm::vec3 sunDir;
    glm::mat4 sunView;
    glm::mat4 sunProj;
    glm::mat4 sunSpaceMatrix;

    unsigned int depthProgram;
    unsigned int depthFBO;
    unsigned int depthMap;

    void updateSun(float time);

    void renderShadow(unsigned int depthProgram, const std::vector<Level*>& levels, int screenW, int screenH, int shadowSize = SHADOW_SIZE);
};

Sun createSun(int shadowSize = SHADOW_SIZE);
