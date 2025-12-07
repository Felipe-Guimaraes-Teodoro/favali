#include "geometry.h"
#include "raycast.h"
#include "glm.hpp"

glm::vec3 MeshTriangle::getTriangleNormal(){

    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;

    return glm::normalize(glm::cross(ab, ac));
}


glm::vec3 closestPointOnTriangle(const glm::vec3& p, MeshTriangle tri) {

    glm::vec3 ab = tri.b - tri.a;
    glm::vec3 ac = tri.c - tri.a;
    glm::vec3 ap = p - tri.a;

    float d1 = glm::dot(ab, ap);
    float d2 = glm::dot(ac, ap);

    if(d1 <= 0.0f && d2 <= 0.0f) return tri.a;

    glm::vec3 bp = p - tri.b;
    float d3 = glm::dot(ab, bp);
    float d4 = glm::dot(ac, bp);

    if(d3 >= 0.0f && d4 <= d3) return tri.b;

    float vc = d1 * d4 - d3 * d2;
    if(vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return tri.a + v * ab;
    }

    glm::vec3 cp = p - tri.c;
    float d5 = glm::dot(ab, cp);
    float d6 = glm::dot(ac, cp);

    if(d6 >= 0.0f && d5 <= d6) return tri.c;

    float vb = d5 * d2 - d1 * d6;
    if(vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return tri.a + w * ac;
    }

    float va = d3 * d6 - d5 * d4;
    if(va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return tri.b + w * (tri.c - tri.b);
    }

    glm::vec3 n = glm::normalize(glm::cross(ab, ac));
    float dist = glm::dot(p - tri.a, n);
    return p - dist * n;
}

AABB makeAABB_from_ray(const Ray& r) {
    glm::vec3 end = r.origin + r.direction * r.tMax;

    AABB b;
    b.min = glm::min(r.origin, end);
    b.max = glm::max(r.origin, end);
    return b;
}
