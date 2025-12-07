#include "raycast.h"

bool Ray::hitTriangle(const MeshTriangle& t, float& outT) {
    const float EPS = 0.00001f;

    glm::vec3 e1 = t.b - t.a;
    glm::vec3 e2 = t.c - t.a;

    glm::vec3 p = glm::cross(direction, e2);
    float det = glm::dot(e1, p);

    if (fabs(det) < EPS) return false; // parallel

    float invDet = 1.0f / det;

    glm::vec3 s = origin - t.a;
    float u = glm::dot(s, p) * invDet;
    if (u < 0 || u > 1) return false;

    glm::vec3 q = glm::cross(s, e1);
    float v = glm::dot(direction, q) * invDet;
    if (v < 0 || u + v > 1) return false;

    float tHit = glm::dot(e2, q) * invDet;
    if (tHit < 0 || tHit > tMax) return false;

    outT = tHit;
    return true;
}
