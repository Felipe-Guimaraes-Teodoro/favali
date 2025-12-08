#pragma once

#include "shapes.h"
#include <memory>

typedef struct {
    float damage;
    float lifetime;
    float speed;
} BulletTemplate;

typedef struct {
    std::unique_ptr<Shape> shape;
    glm::vec3 dir;
    float damage;
    float lifetime;
    float speed;

    void update_bullet(float dt);
} Bullet;

Bullet create_bullet(const glm::vec3& muzzleWorld, const glm::vec3& forward);

BulletTemplate create_bullet_template();
