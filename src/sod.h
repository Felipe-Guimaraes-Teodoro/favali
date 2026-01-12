#pragma once

#include "glm.hpp"

using glm::vec3;

// second order dynamics
typedef struct {
    float k1, k2, k3;
    vec3 xp, y, yd;

    // critical timestep threshold where the simulation would
    // become unstable past it
    float t_critical;
} SOD;

SOD create_sod(float f, float z, float r, vec3 x0);
void update_sod(SOD& sod, float timestep, vec3 x);
