#pragma once

#include "bullet.h"
#include "player.h"
#include "mesh.h"
#include "glm.hpp"
#include "SDL3/SDL.h"
#include "ik.h"

typedef struct{
    std::unique_ptr<Shape> shape;
    float last_shot;
    std::vector<Bullet> bullets;
    float recoil_timer;
    
    std::vector<Transform> bullet_holes;
    InstancedMesh bullet_hole_mesh;
    unsigned int bullet_hole_tex;

    glm::vec3 muzzle_back_sample; // arbitrary position behind muzzle_pos that is used to get the direction of the bullet
    glm::vec3 muzzle_pos; // position where the bullet is actually instantiated
    bool is_shooting;
    BulletTemplate bullet_template;
    float cooldown;
    float spread;
    unsigned int bullets_per_shot;

    bool aiming;

    void update(float dt, Camera& camera, IkController& controller, Player& player, std::vector<BVHNode*> worldBVHs);

    void draw(unsigned int program, unsigned int instanced_program, Camera& camera);
} Gun;

Gun make_gun(glm::vec3 muzzle_back_sample, glm::vec3 muzzle_pos, float cooldown, float spread, unsigned int bullets_per_shot, unsigned int bullet_hole_tex);
