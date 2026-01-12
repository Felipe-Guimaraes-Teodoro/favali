#pragma once

#include "glm.hpp"
#include "geometry.h"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
    float tMax;
    
    bool hitTriangle(const MeshTriangle& t, float& outT);
};
