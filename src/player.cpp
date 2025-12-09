#include <vector>
#include <algorithm>
#include <stdio.h>

#include "player.h"
#include "gizmos.h"
#include "geometry.h"
#include "sod.h"
#include "raycast.h"
#include "SDL3/SDL.h"

void PlayerCollider::update(float dt) {
    // im gonna be honest playercollider is pretty much just a wrapper for some variables at this point...
}

/// force to then be applied in subsequent update calls
void PlayerCollider::apply_force(vec3 force) {
    resulting_force += force;
}

PlayerCollider default_player_collider() {
    PlayerCollider coll = {};

    coll.radius = 1.0; // make it a cillinder in the future
    coll.center = vec3(0, 0.5, 0); // usually just player.position, that's why we have offset

    return coll;
}

void Player::update(float dt, bool lock_cursor, float sensitivity, Camera& camera) {
    player_movement(dt, lock_cursor, sensitivity, camera);
    collider.update(dt);
    collider.center = position;
}

void Player::solve_collisions(std::vector<BVHNode*> worldBVHs){
    std::vector<MeshTriangle> candidates;
    AABB query;
    query.min = position - glm::vec3(collider.radius);
    query.max = position + glm::vec3(collider.radius); // create bounding box roughly the size of the player

    for (int i = 0; i < worldBVHs.size(); i++){
        bvhQuery(worldBVHs[i], query, candidates); // get possible triangles to intersect

        for (auto& tri : candidates) {
            // closest tri to point
            glm::vec3 closest = closestPointOnTriangle(position, tri);

            glm::vec3 triNormal = tri.getTriangleNormal();

            push_gizmo(Shapes::Sphere, Transform(closest, vec3(0.1)), {0.0, 0.0, 1.0, 1.0});

            glm::vec3 diff = position - closest;
            float dist = length(diff);

            if (dist < collider.radius) {
                float push = collider.radius - dist;

                // push player by the tri's normal
                position += triNormal * push;
            }
        }
        candidates.clear();
    }

    push_gizmo(Shapes::Cube, query, {1.0, 1.0, 0.0, 1.0});

    AABB feet;
    makeAABB_from_ray((Ray) {
        .origin = position,
        .direction = -UP,
        .tMax = collider.radius * 2.0,
    });

    push_gizmo(Shapes::Cube, feet, {1.0, 0.0, 0.0, 1.0});

    for (int i = 0; i < worldBVHs.size(); i++){
        bvhQuery(worldBVHs[i], feet, candidates);

        for (auto& tri : candidates) {
            glm::vec3 feetCenter = (feet.min + feet.max) * 0.5f;
            glm::vec3 closest = closestPointOnTriangle(feetCenter, tri);

            push_gizmo(Shapes::Sphere, Transform(closest, vec3(0.1)), {0.0, 1.0, 0.0, 1.0});

            glm::vec3 diff = feetCenter - closest;
            float dist = length(diff);

            if (dist < collider.radius) {
                grounded = true;
                printf("Player is grounded\n");
            }
        }
        candidates.clear();
    }
}

void Player::player_movement(float dt, bool lock_cursor, float sensitivity, Camera& camera){
    static SOD cam_sod = create_sod(1.5, 0.5, 0.5, position);
    last_pos = position;
    
    const bool *state = SDL_GetKeyboardState(NULL);

    float mouse_x, mouse_y;
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
    
    camera.mouse_view(lock_cursor, mouse_x, mouse_y, sensitivity);

    wishdir = vec3(0);
    if (state[SDL_SCANCODE_W]){
        wishdir += camera.front;
    }
    if (state[SDL_SCANCODE_S]){
        wishdir -= camera.front;
    }
    if (state[SDL_SCANCODE_A]){
        wishdir += camera.right;
    }
    if (state[SDL_SCANCODE_D]){
        wishdir -= camera.right;
    }
    if (state[SDL_SCANCODE_SPACE]){
        jump_current = jump_force;

        position += UP*  dt * speed; // temp
        // grounded = false;
    }

    if (state[SDL_SCANCODE_LCTRL]){ // temp
        position -= UP*  dt * speed; // temp
        // grounded = false;
    }

    wishdir.y = 0; // only move on the XZ plane with WASD
    if (glm::length(wishdir) > 0.1) {
        wishdir = glm::normalize(wishdir) * speed;
    }
    position += wishdir * dt;

    // jump_force += GRAVITY.y * damping;
    // if (jump_force < 0) {
    //     jump_force = 0;
    // }
    // velocity.y += (GRAVITY.y + jump_force) * dt;
    // velocity *= damping;

    // if (grounded){
    //     jump_current = 0.f;
    //     velocity.y = 0;
    // }

    // velocity *= damping;
    position += velocity * dt;

    Ray grounded_ray = (Ray){
        .origin = position,
        .direction = -UP,
        .tMax = collider.radius*0.55f, // go half of player's size
    };

    // printf("player vel: %f\n", current_speed);

    update_sod(cam_sod, dt, position + head_ofs);
    camera.position = cam_sod.y;
    current_speed = glm::length(last_pos - position);

    camera.update();
}

Player create_player() {
    Player player = {};
    player.collider = default_player_collider();
    player.head_ofs = vec3(0.0, 0.5, 0.0);
    player.speed = 5.8;
    player.damping = 0.9;
    player.grounded = true;
    player.jump_force = 100.5;

    return player;
}
