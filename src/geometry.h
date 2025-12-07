#pragma once

#include "glm.hpp"

struct Ray; // forward declaration so there's no circular definition (raycast.h also includes geometry)

struct MeshTriangle {
    glm::vec3 a, b, c;
    
    glm::vec3 getTriangleNormal();
};


struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

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
