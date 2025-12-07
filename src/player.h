/* 
its probably better to do this after establishing some physics library/architecture
on the codebase cause i freaking love physically based movement
*/

#pragma once

#include "glm.hpp"
using glm::vec3;

constexpr glm::vec3 GRAVITY = glm::vec3(0.0f, -10.0f, 0.0f);

typedef struct {
    vec3 size;
    vec3 center;
    // shape: box

    vec3 resulting_force;

    void update(float dt) {
        // hardcoded floor plane
        if (center.y - size.y < 0.0) {
            center.y = 0.0 + size.y;
        }

        center += GRAVITY * dt;
        center += resulting_force * dt;
    }

    /// to then be applied in subsequent update calls
    void apply_force(vec3 force) {
        resulting_force += force;
    }
} PlayerCollider;

PlayerCollider default_player_collider() {
    PlayerCollider coll = {};

    coll.size = vec3(0.2, 1.0, 0.2);
    coll.center = vec3(0, 0.5, 0);

    return coll;
}

typedef struct {
    vec3 head;
    vec3 head_ofs;

    PlayerCollider collider;

    void update(float dt) {
        collider.update(dt);
        head = collider.center + head_ofs;
    }
} Player;

Player create_player() {
    Player player = {};
    player.head = vec3(0);
    player.collider = default_player_collider();
    // collider height is 0.5
    player.head_ofs = vec3(0.0, 0.5 + 0.2, 0.0);
    
    return player;
}