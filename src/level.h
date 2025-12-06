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