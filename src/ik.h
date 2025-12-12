#pragma once

#include "glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/quaternion.hpp"

#include "level.h"

struct IkNode {
    // JointKind joint;

    glm::vec3 origin;
    glm::vec3 end;

    glm::quat rotation;
    glm::vec3 axis; // allowed rotation axis

    float length;

    IkNode* next = nullptr;
    IkNode* previous = nullptr;
};

IkNode* create_ik_node(
    float length = 1.0, 
    glm::vec3 axis = {1.0, 0.0, 0.0}, 
    glm::quat rotation = glm::quat(1,0,0,0)
);

struct IkController {
    IkNode *root;
    IkNode *leaf;

    glm::vec3 origin;
    glm::vec3 goal;

    glm::quat visual_leaf_rot;
    glm::quat visual_root_rot;

    ~IkController();

    // FABRIK
    void update(float tolerance = 0.01f, int max_iter = 1, float alpha = 0.01f);

    void set_arm_transform(Level* arm);
    void draw_dbg();

    void push_node(IkNode* node);
};

IkController create_ik_controller(glm::vec3 origin);