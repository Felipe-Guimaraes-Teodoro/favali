#pragma once

#include "shapes.h"
#include <memory>

struct Bullet {
    std::unique_ptr<Shape> shape;
    glm::vec3 dir;

    void update_bullet(float speed, float dt);
};

Bullet create_bullet(const glm::vec3& muzzleWorld, const glm::vec3& forward);
