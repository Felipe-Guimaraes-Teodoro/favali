#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm.hpp"
#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"
#include "gtc/matrix_transform.hpp"

struct Transform {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    Transform(glm::vec3 pos, glm::quat rot, glm::vec3 scl){
        position = pos;
        rotation = rot;
        scale = scl;
    }
    
    static Transform empty() {
        return Transform(
            glm::vec3(0),
            glm::quat(1,0,0,0),
            glm::vec3(1)
        );
    }

    glm::mat4 getModelMat() const {
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, position);
        model *= glm::mat4_cast(rotation);
        model = glm::scale(model, glm::vec3(scale));

        return model;
    }

};