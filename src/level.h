#pragma once

#include <vector>

#include "shapes.h"

using std::vector;

typedef struct {
    vector<Shape> shapes;
    // vector<Collider> colliders;
    // vector<Entity> entities; // shapes that move

} Level;

Level *create_level();

void merge_level_shapes(Level* level);

// void create_level_collider() {}