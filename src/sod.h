#include <math.h>

#include "glm.hpp"

using glm::vec3;

/// Second order dynamics
/// 
typedef struct {
    float k1, k2, k3;

    vec3 xp, y, yd;

    // critical timestep threshold where the simulation would
    // become unstable past it
    float t_critical;
} SOD;

SOD create_sod(float f, float z, float r, vec3 x0) {
    SOD sod = {};

    sod.k1 = z / (M_PI * f);
    sod.k2 = 1.0 / ((2.0 * M_PI * f) * (2.0 * M_PI * f));
    sod.k3 = r * z / (2.0 * M_PI * f);
    
    sod.xp = x0;
    sod.y = x0;
    sod.yd = vec3(0.0, 0.0, 0.0);

    sod.t_critical = (sqrt(4.0*sod.k2 + sod.k1 * sod.k1) - sod.k1) * 0.8; // multiply by an arbitrary value to be safe

    return sod;
}

// todo: decide if updates should be NAME_update() or update_NAME()
// yes i aint deciding just that now
// naming is hard
void update_sod(SOD& sod, float timestep, vec3 x) {
    vec3 xd = (x - sod.xp) / timestep;
    sod.xp = x;

    int iterations = ceil(timestep / sod.t_critical); // take extra iterations if t > tcrit
    timestep = timestep / iterations; // lower timesteps

    for (int i = 0; i < iterations; i++) {
        sod.y = sod.y + timestep * sod.yd;
        sod.yd = sod.yd + timestep * (x + sod.k3*xd - sod.y - sod.k1*sod.yd) / sod.k2;
    }
}