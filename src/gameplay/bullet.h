#pragma once

#include <memory>
#include "shapes.h"
#include "BVH.h"

typedef struct {
    float damage;
    float lifetime;
    float speed;
} BulletTemplate;

typedef struct {
    std::unique_ptr<Shape> shape;
    glm::vec3 dir; // always normalize!
    glm::vec3 normal;
    float damage;
    float lifetime;
    float speed;

    void update_bullet(float dt);

    bool handle_collisions(float dt, std::vector<BVHNode*> worldBVHs);
} Bullet;

Bullet create_bullet(const glm::vec3& muzzleWorld, const glm::vec3& forward);


BulletTemplate create_bullet_template();
