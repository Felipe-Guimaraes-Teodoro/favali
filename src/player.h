/* 
its probably better to do this after establishing some physics library/architecture
on the codebase cause i freaking love physically based movement
*/

#pragma once

#include "glm.hpp"
#include "BVH.h"
#include "camera.h"

using glm::vec3;

constexpr glm::vec3 GRAVITY = glm::vec3(0.0f, -10.0f, 0.0f);

typedef struct {
    vec3 center;
    vec3 resulting_force;
    float radius;
    // shape: box

    void update(float dt);

    void apply_force(vec3 force);
} PlayerCollider;

PlayerCollider default_player_collider();

typedef struct {
    vec3 head_ofs;
    vec3 position;
    float speed;

    PlayerCollider collider;

    void update(float dt, bool lock_cursor, float sensitivity, Camera& camera);

    void player_movement(float dt, bool lock_cursor, float sensitivity, Camera& camera);

    void solve_collisions(std::vector<BVHNode*> worldBVHs);
} Player;

Player create_player();
