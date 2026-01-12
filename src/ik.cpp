#include "ik.h"

#include <stdio.h>

#include "transform.h"
#include "gtx/vector_angle.hpp"

#include "gizmos.h"

using glm::vec3;
using glm::mat4;
using glm::vec4;
using glm::quat;

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
    // node->axis = axis;
    
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
                glm::quat r = rotate_from_to(
                    current->end - current->origin, 
                    goal - current->origin
                    // current->axis
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

void IkController::set_arm_transform(Model* arm, Camera& camera) {
    IkNode* cur = root;
    int i = 0;

    const static quat y90 = glm::angleAxis(glm::radians(90.0f), vec3(1.0, 0.0, 0.0));

    while (cur) {
        vec3 dir = cur->end - cur->origin;
        if (cur->length < 1e-6f) dir = vec3(1, 0, 0);
        dir = normalize(dir);

        vec3 forward = dir;
        vec3 right = glm::normalize(glm::cross(camera.up, forward));
        if (glm::length(right) < 1e-6f) {
            right = normalize(cross(vec3(1,0,0), forward));
        }
        vec3 up = glm::cross(forward, right);

        glm::mat3 rot_mat(right, up, forward);
        quat rot = glm::quat_cast(rot_mat);

        vec3 mid = cur->origin + dir * 0.5f * cur->length;

        arm->shapes[i].transform.rotation = rot * y90;
        arm->shapes[i].transform.position = mid;

        if (i == 0) {
            visual_root_rot = rot;
        }
        if (i == arm->shapes.size() - 1) {
            visual_leaf_rot = rot;
        }

        cur = cur->next;
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