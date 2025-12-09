#pragma once

#include "shapes.h"
#include "transform.h"
#include "BVH.h"

void init_gizmos();

void push_gizmo(Shapes shape, Transform t, glm::vec4 col = {1.0, 1.0, 1.0, 1.0});
void push_gizmo(Shapes shape, AABB aabb, glm::vec4 col = {1.0, 1.0, 1.0, 1.0});
void push_gizmo_n_frames(Shapes shape, Transform t, int n, glm::vec4 col = {1.0, 1.0, 1.0, 1.0});
void pop_gizmo();

void render_gizmos(Camera& cam);