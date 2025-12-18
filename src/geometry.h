#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm.hpp"
// #include "shapes.h"
 
struct Ray; // forward declaration to not fuck shit up

struct MeshTriangle {
    glm::vec3 a, b, c;
    
    glm::vec3 getTriangleNormal();
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

typedef struct {
    glm::vec3 normal;
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

inline AABB makeAABB(const MeshTriangle& t){
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

glm::vec3 closestPointOnTriangle(const glm::vec3& p, MeshTriangle tri);

AABB makeAABB_from_ray(const Ray& r);

inline float aabb_distance_from_plane(const Plane& plane, const AABB& aabb) {
    const glm::vec3 c = (aabb.min + aabb.max) * 0.5f; // center
    const glm::vec3 e = (aabb.max - aabb.min) * 0.5f; // half extents

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

// utility to create a plane from three points
inline Plane makePlane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
    float distance = glm::dot(normal, a);
    return { normal, distance };
}

glm::quat rotate_from_to(const glm::vec3& from, const glm::vec3& to);

// AABB makeAABB_from_shape(const Shape& shape);