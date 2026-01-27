#pragma once

// #include "shapes.h"
#include "glm.hpp"
using glm::vec3;
 
struct Ray; // forward declaration to not fuck shit up

struct MeshTriangle {
    vec3 a, b, c;
    
    vec3 get_tri_normal() const;
};

struct AABB {
    vec3 min;
    vec3 max;
};

typedef struct {
    vec3 normal;
    float distance;
} Plane;

typedef struct {
    Plane top;
    Plane bottom;
    Plane right;
    Plane left;
    Plane far;
    Plane near;
} Frustum;

inline AABB create_AABB(const MeshTriangle& t){
    AABB b;
    b.min = glm::min(glm::min(t.a, t.b), t.c);
    b.max = glm::max(glm::max(t.a, t.b), t.c);
    return b;
}

inline bool intersects(const AABB& a, const AABB& b){
    return (a.min.x <= b.max.x && a.max.x >= b.min.x)
        && (a.min.y <= b.max.y && a.max.y >= b.min.y)
        && (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

vec3 closest_point_on_tri(const vec3& p, MeshTriangle tri);

AABB create_AABB_from_ray(const Ray& r);

inline float aabb_distance_from_plane(const Plane& plane, const AABB& aabb) {
    const vec3 c = (aabb.min + aabb.max) * 0.5f; // center
    const vec3 e = (aabb.max - aabb.min) * 0.5f; // half extents

    const float r =
        e.x * std::abs(plane.normal.x) +
        e.y * std::abs(plane.normal.y) +
        e.z * std::abs(plane.normal.z);

    const float s = glm::dot(plane.normal, c) - plane.distance;

    return s + r;
}

bool is_aabb_on_frustum(const Frustum& frustum, const AABB& aabb);
bool is_point_on_aabb(const AABB& aabb, const glm::vec3& point);
bool is_line_on_aabb(const AABB& aabb, const glm::vec3& start, const glm::vec3& end);
bool ray_intersects_aabb(const AABB& box, const Ray& ray, float& outT);

Plane create_plane(const vec3& a, const vec3& b, const vec3& c);

glm::quat rotate_from_to(const glm::vec3& from, const glm::vec3& to);

// AABB makeAABB_from_shape(const Shape& shape);