#pragma once

#include "bullet.h"
#include "glm.hpp"
#include "SDL3/SDL.h"

typedef struct{
    std::unique_ptr<Shape> shape;
    float last_shot;
    std::vector<Bullet> bullets;
    float recoil_timer;
    
    glm::vec3 muzzle_pos;
    bool is_shooting;
    BulletTemplate bullet_template;
    float cooldown;
    float spread;
    unsigned int bullets_per_shot;

    void update(float dt, Camera& camera);

    void draw(unsigned int program, Camera& camera);
} Gun;

Gun make_gun(glm::vec3 muzzle_pos, float cooldown, float spread, unsigned int bullets_per_shot);
