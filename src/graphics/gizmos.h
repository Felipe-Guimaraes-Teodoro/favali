#pragma once

#include "shapes.h"
#include "transform.h"
#include "BVH.h"

void init_gizmos();

void push_gizmo(Shapes shape, Transform t, int n = 1, glm::vec4 col = {1.0, 1.0, 1.0, 1.0});
void push_gizmo(Shapes shape, AABB aabb, int n = 1, glm::vec4 col = {1.0, 1.0, 1.0, 1.0});
void pop_gizmo();

void render_gizmos(Camera& cam);
void end_frame_gizmos();