#pragma once

#include "glm.hpp"
#include "glad/glad.h"
#include <vector>

#define SHADOW_SIZE 4096

class Level;

struct Sun{
    glm::vec3 sunDir;
    glm::mat4 sunView;
    glm::mat4 sunProj;
    glm::mat4 sunSpaceMatrix;

    unsigned int depthProgram;
    unsigned int depthFBO;
    unsigned int depthMap;

    void update_sun(float time);

    void render_shadow(unsigned int depthProgram, const std::vector<Level*>& levels, int screenW, int screenH, int shadowSize = SHADOW_SIZE);
};

Sun create_sun(int shadowSize = SHADOW_SIZE);
