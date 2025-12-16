#pragma once

#include <vector>

#include "shapes.h"
#include "geometry.h"
#include "light.h"

using std::vector;

typedef struct {
    Lights lights;

    vector<StaticShape> shapes;

    // todo: shapes -> models
    // octree (for pathfinding, culling)
    // vector<Collider> colliders;
    // vector<Entity> entities; // shapes that move

} Level;

typedef struct {
    Lights lights;

    vector<Shape> shapes;
} Model;

Level *create_level();
Model *create_model();

void draw_level(Level *l, Camera& cam, unsigned int program, bool should_cull = false);
void draw_model(Model *l, Camera& cam, unsigned int program);

void merge_level_shapes(Level* level);

// void create_level_collider() {}

// get all mesh triangles out of the map to create BVH structure for collisions
std::vector<MeshTriangle> get_level_tris(StaticShape *shape);
