#pragma once

#include <vector>
using std::vector;

#include "glm.hpp"
using glm::vec3;
using glm::ivec3;
#include "BVH.h"

typedef struct NavNode {
    vector<struct NavNode*> neighbors;
    
    ivec3 pos;
} NavNode;

typedef struct {    
    vector<NavNode*> nodes;
} NavGraph;

extern NavGraph *global_navGraph;

NavGraph* create_navgraph(vector<BVHNode*>& bvh);

void visualize_nodes(vector<NavNode*> nodes);
void visualize_path(vector<NavNode*> nodes);

typedef struct {
    vec3 position;
    vec3 goal;
    bool cant_find_route;

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

    vector<NavNode*> *path;
} Npc;

Npc create_npc();

void npc_solve_collisions(Npc& npc, vector<BVHNode*> worldBVHs);
void update_npc(Npc& npc, float dt);

void set_npc_goal(vec3 goal);