#pragma once

#include "shapes.h"

struct Bullet{
    Shape shape;
    glm::vec3 dir;

    Bullet() 
        : shape(make_shape(Shapes::Sphere)), dir(0.0f, 0.0f, 0.0f) {
    }

    void update_bullet(float speed, float dt);
};