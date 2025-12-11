#include "ik.h"

#include <stdio.h>

#include "transform.h"
#include "gtx/vector_angle.hpp"

#include "gizmos.h"

using glm::vec3;
using glm::mat4;
using glm::vec4;
using glm::quat;

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

quat rotate_from_to_constrained(const vec3& from, const vec3& to, const vec3& allowedAxis) {
    vec3 f = glm::normalize(from);
    vec3 t = glm::normalize(to);

    vec3 axis = glm::cross(f, t);

    // project axis onto allowed axis
    axis = glm::dot(axis, allowedAxis) * allowedAxis;
    if (glm::length2(axis) < 0.0001f)
        return glm::identity<quat>();

    float cosTheta = glm::dot(f, t);
    float angle = glm::acos(glm::clamp(cosTheta, -1.0f, 1.0f));

    return glm::angleAxis(angle, glm::normalize(axis));
}

IkNode* create_ik_node(
    float length,
    vec3 axis,
    quat rotation
) {
    IkNode* node = new IkNode();
    node->length = length;
    node->rotation = rotation;
    node->previous = nullptr;
    node->next = nullptr;
    node->origin = glm::vec3(0.0f);
    node->end = glm::vec3(0.0f, length, 0.0f);
    node->axis = axis;
    
    return node;
}

IkController::~IkController() {
    IkNode* current = root;
    while (current) {
        IkNode* next = current->next;
        delete current;
        current = next;
    }
}

// FABRIK
void IkController::update(float tolerance, int max_iter, float alpha) {
    if (!root) return;

    int i = 0;

    vec3 root_origin = root->origin;

    while (glm::distance(leaf->end, goal) > tolerance && i < max_iter) {
        IkNode* current = leaf;
        while (current != nullptr) {
            if (glm::length2(goal - current->origin) > 0.0001f && glm::length2(current->end - current->origin) > 0.0001f) {
                glm::quat r = rotate_from_to_constrained(
                    current->end - current->origin, 
                    goal - current->origin,
                    current->axis
                );
                
                // dont rotate all at once
                r = glm::slerp(glm::quat(1,0,0,0), r, alpha);
                current->rotation = r * current->rotation;
            }

            current = current->previous;
        }

        current = root;
        vec3 pos = root_origin;
        while (current != nullptr) {
            current->origin = pos;
            current->end = pos + current->rotation * UP * current->length;
            pos = current->end;
            current = current->next;
        }

        i++;
    }
}

void IkController::draw_dbg() {
    IkNode* cur = root;

    while (cur) {
        push_gizmo(Shapes::Sphere, Transform(cur->origin, vec3(0.1f)));

        // todo: put this on its own fn
        // (figuring out transform from ik node)
        glm::vec3 dir = cur->end - cur->origin;
        float length = glm::length(dir);
        dir = glm::normalize(dir);
        glm::quat rot = rotate_from_to(glm::vec3(0.0f, 0.0f, 1.0f), dir);
        glm::vec3 scale(0.1f, 0.1f, length);

        glm::vec3 mid = cur->origin + dir * (length * 0.5f);

        push_gizmo(Shapes::Cube, Transform(mid, rot, scale));

        cur = cur->next;
    }

    if (root) {
        IkNode* end = root;
        while (end->next) end = end->next;
        push_gizmo(Shapes::Sphere, Transform(end->end, vec3(0.1f)));
    }
}


void IkController::push_node(IkNode* node) {
    if (!root) {
        root = node;
        leaf = node;
        node->previous = nullptr;
        node->next = nullptr;
        node->origin = origin;
        node->end = node->origin + node->rotation * UP * node->length;
        return;
    }

    IkNode* last = leaf;
    last->next = node;
    node->previous = last;
    node->next = nullptr;

    node->origin = last->end;
    node->end = node->origin + node->rotation * UP * node->length;

    leaf = node;
}

IkController create_ik_controller(vec3 origin) {
    IkController controller = {
        .root = nullptr,
        .leaf = nullptr,
        .origin = origin,
    };
    
    return controller;
}