#include "geometry.h"
#include "raycast.h"
#include "glm.hpp"
#include "stdio.h"
#include "gtx/vector_angle.hpp"
using glm::quat;
using glm::vec3;

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

bool is_aabb_on_frustum(const Frustum& frustum, const AABB& aabb) {
    if (aabb_distance_from_plane(frustum.near, aabb) < FLT_EPSILON) return false;
    if (aabb_distance_from_plane(frustum.far, aabb) < FLT_EPSILON) return false;
    if (aabb_distance_from_plane(frustum.left, aabb) < FLT_EPSILON) return false;
    if (aabb_distance_from_plane(frustum.right, aabb) < FLT_EPSILON) return false;
    if (aabb_distance_from_plane(frustum.top, aabb) < FLT_EPSILON) return false;
    if (aabb_distance_from_plane(frustum.bottom, aabb) < FLT_EPSILON) return false;

    return true;
}

bool is_point_on_aabb(const AABB& aabb, const glm::vec3& point) {
    if (
        point.x > aabb.min.x && 
        point.y > aabb.min.y && 
        point.z > aabb.min.z &&
        point.x < aabb.max.x && 
        point.y < aabb.max.y && 
        point.z < aabb.max.z 
    ) {
        return true;
    }

    return false;
}

bool is_line_on_aabb(const AABB& aabb, const glm::vec3& start, const glm::vec3& end) {
    glm::vec3 invDir = 1.0f / (end - start);
    glm::vec3 t0s = (aabb.min - start) * invDir;
    glm::vec3 t1s = (aabb.max - start) * invDir;

    glm::vec3 tmin = glm::min(t0s, t1s);
    glm::vec3 tmax = glm::max(t0s, t1s);

    float t_enter = std::max(std::max(tmin.x, tmin.y), tmin.z);
    float t_exit  = std::min(std::min(tmax.x, tmax.y), tmax.z);

    return t_enter <= t_exit && t_exit >= 0.0f && t_enter <= 1.0f;
}

glm::quat rotate_from_to(const vec3& from, const vec3& to) {
    vec3 f = glm::normalize(from);
    vec3 t = glm::normalize(to);

    float cosTheta = glm::dot(f, t);

    // nearly opposite
    if (cosTheta < -0.999) {
        vec3 axis = glm::cross(vec3(1.0f, 0.0f, 0.0f), f);
        if (glm::length2(axis) < 0.0001f)
            axis = glm::cross(vec3(0.0f, 1.0f, 0.0f), f);
        axis = glm::normalize(axis);
        return glm::angleAxis(glm::pi<float>(), axis);
    }

    // nearly same
    if (cosTheta > 0.999)
        return glm::identity<quat>();
    vec3 axis = glm::cross(f, t);
    float angle = glm::acos(glm::clamp(cosTheta, -1.0f, 1.0f));

    return glm::angleAxis(angle, glm::normalize(axis));
}

/*
AABB makeAABB_from_shape(const Shape& shape) {
    AABB b;
    printf("makeAABB_from_shape not implemented\n");
    return b;
}
*/
