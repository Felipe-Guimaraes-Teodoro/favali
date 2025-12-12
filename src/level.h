#pragma once

#include <vector>

#include "shapes.h"
#include "geometry.h"
#include "light.h"

using std::vector;

typedef struct {
    Lights lights;
    vector<Shape> shapes;
    // vector<Collider> colliders;
    // vector<Entity> entities; // shapes that move

} Level;

Level *create_level();

void draw_level(Level *l, Camera& cam, unsigned int program);

void merge_level_shapes(Level* level);

// void create_level_collider() {}

// get all mesh triangles out of the map to create BVH structure for collisions
std::vector<MeshTriangle> get_level_tris(Shape *shape);
