#pragma once

#include "geometry.h"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
    float tMax;
    
    bool hit_triangle(const MeshTriangle& t, float& outT);
};
