#pragma once

#include <unordered_map>
using std::unordered_map;
#include <vector>
using std::vector;

#include "glm.hpp"
using glm::vec3;
using glm::ivec3;
#include "BVH.h"

struct ivec3_hash {
    size_t operator()(const glm::ivec3& k) const noexcept {
        size_t hx = std::hash<int>{}(k.x);
        size_t hy = std::hash<int>{}(k.y);
        size_t hz = std::hash<int>{}(k.z);
        return hx ^ (hy << 1) ^ (hz << 2);
    }
};

struct ivec3_equal {
    bool operator()(const glm::ivec3& a, const glm::ivec3& b) const noexcept {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};


typedef struct NavNode {
    vector<ivec3> neighbors;
} NavNode;

typedef struct {    
    unordered_map<ivec3, NavNode*, ivec3_hash, ivec3_equal> nodes;
} NavGraph;

extern NavGraph *global_navGraph;

NavGraph* create_navgraph(vector<BVHNode*>& bvh);

void visualize_nodes(const NavGraph* graph);
void visualize_path(const std::vector<glm::ivec3>& path);

typedef struct {
    vec3 old_position;
    vec3 position;
    vec3 goal; // could be a a player, some cover, away from the player, etc
    bool cant_find_route; // more like "should_recreate_path"

    struct {
        vec3 center;
        float radius;
    } collider;

    vec3 wishdir;
    vec3 velocity;
    vec3 accel;
    float speed;

    float jump_force;
    float last_jumped;
    bool grounded;

    vector<ivec3> path;
    int path_idx;
    float last_pathfounded;
    bool on_goal;
} Npc;

Npc create_npc();

void npc_solve_collisions(Npc& npc, vector<BVHNode*> worldBVHs);
void update_npc(Npc& npc, float dt);

void set_npc_goal(vec3 goal);