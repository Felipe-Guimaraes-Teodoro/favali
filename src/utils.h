#pragma once

#include <random>

std::uniform_real_distribution<float> dist01(0.f, 1.f);

inline float randf(float a, float b) {
    return a + (b - a) * (float(rand()) / float(RAND_MAX));
}

glm::vec3 random_on_cone(
    const glm::vec3& dir,
    float angle_rad,
    std::mt19937& gen
) {
    std::uniform_real_distribution<float> dist01(0.f, 1.f);

    float u = dist01(gen);
    float v = dist01(gen);

    float theta = 2.f * M_PI * u;
    float phi = acos(1.f - v * (1.f - cos(angle_rad)));

    glm::vec3 local(
        sin(phi) * cos(theta),
        sin(phi) * sin(theta),
        cos(phi)
    );

    glm::quat q = glm::rotation(glm::vec3(0, 0, 1), dir);
    return glm::normalize(q * local);
}
