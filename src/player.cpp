#include <vector>
#include <algorithm>
#include <stdio.h>

#include "player.h"
#include "gizmos.h"
#include "geometry.h"
#include "sod.h"
#include "raycast.h"
#include "SDL3/SDL.h"
#include "audio.h"

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

            push_gizmo(Shapes::Sphere, Transform(closest, vec3(0.1)), 1, {0.0, 0.0, 1.0, 1.0});

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

    push_gizmo(Shapes::Cube, query, 1, {1.0, 1.0, 0.0, 1.0});

    AABB feet;
    makeAABB_from_ray((Ray) {
        .origin = position,
        .direction = -UP,
        .tMax = collider.radius * 2.0f,
    });

    push_gizmo(Shapes::Cube, feet, 1, {1.0, 0.0, 0.0, 1.0});

    grounded = false;

    for (int i = 0; i < worldBVHs.size(); i++){
        bvhQuery(worldBVHs[i], feet, candidates);

        for (auto& tri : candidates) {
            glm::vec3 feetCenter = (feet.min + feet.max) * 0.5f;
            glm::vec3 closest = closestPointOnTriangle(feetCenter, tri);

            push_gizmo(Shapes::Sphere, Transform(closest, vec3(0.1)), 1, {0.0, 1.0, 0.0, 1.0});

            glm::vec3 diff = feetCenter - closest;
            float dist = length(diff);

            if (dist < collider.radius * 0.5) {
                grounded = true;
            }
        }
        candidates.clear();
    }
}

void Player::player_movement(float dt, bool lock_cursor, float sensitivity, Camera& camera){
    static SOD cam_sod = create_sod(2.5, 0.5, 0.0, position);
    last_pos = position;
    
    const bool *state = SDL_GetKeyboardState(NULL);

    float mouse_x, mouse_y;
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
    
    camera.mouse_view(lock_cursor, mouse_x, mouse_y, sensitivity);

    wishdir = vec3(0);
    vec3 zx_front = glm::normalize(vec3(camera.front.x, 0.0, camera.front.z));

    if (state[SDL_SCANCODE_W]){
        wishdir += zx_front;
    }
    if (state[SDL_SCANCODE_S]){
        wishdir -= zx_front;
    }
    if (state[SDL_SCANCODE_A]){
        wishdir += camera.right;
    }
    if (state[SDL_SCANCODE_D]){
        wishdir -= camera.right;
    }
    if (state[SDL_SCANCODE_LSHIFT]){
        if (speed < speed_cap)
            speed += sprint_accel * dt;
        if (speed > speed_cap)
            speed = speed_cap;
    } else 
        speed = speed_base;
    if (state[SDL_SCANCODE_LCTRL]) {
        crouching = true;
    } else 
        crouching = false;

    if (state[SDL_SCANCODE_SPACE] && grounded && last_jumped < 0.0){
        velocity.y = jump_force;
        // push the player enough so it doesnt 
        // get detected as grounded mid jump
        position.y += collider.radius * 0.5;
        last_jumped = 0.1;
        grounded = false;
    }

    wishdir.y = 0; // only move on the XZ plane with WASD
    if (glm::length(wishdir) > 0.1) {
        wishdir = glm::normalize(wishdir) * speed;
    }
    if (!crouching) {
        collider.radius = 1.0;
        velocity.x *= damping;
        velocity.z *= damping;
        velocity += wishdir;
    } else {
        collider.radius = 0.5;
        if (!grounded && dive_boost && last_jumped < 0.0) {
            dive_boost = false;
            velocity.x *= 1.6;
            velocity.y *= 0.7;
            velocity.z *= 1.6;
        }

        if (grounded) {
            position += wishdir * dt;

            velocity.x *= powf(damping, 0.02);
            velocity.z *= powf(damping, 0.02);
        }
        wishdir = vec3(0);
        // fall faster
        velocity.y -= jump_force * dt * 2.5f;
    }

    accel = GRAVITY * dt;
    velocity.y += accel.y;
    last_jumped -= dt;

    if (grounded && last_jumped < 0.0) {
        dive_boost = true;
        // jump_current = 0.f;
        velocity.y = 0.f;
    }

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
    player.position = {0.0, 5.0, 0.0};
    player.head_ofs = vec3(0.0, 0.5, 0.0);
    player.speed = 3.3;
    player.speed_base = player.speed;
    player.speed_cap = player.speed * 1.6 * 50.0;
    player.sprint_accel = 4.0;
    player.damping = 0.6;
    player.grounded = true;
    player.crouching = false;
    player.dive_boost = false;
    player.jump_force = 6.5;
    player.last_jumped = 0;

    return player;
}
