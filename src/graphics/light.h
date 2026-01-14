#pragma once

#include "glm.hpp"

struct Lights {
    struct {
        glm::vec3 v;
        float padding;
    }  position[20]; // 20 is the default maximum allowed lights

    struct {
        glm::vec3 v;
        float padding;
    }  color[20];

    // i'm declaring the padded vectors like this cause
    // it would be visible to the whole codebase otherwise

    int16_t count; // ammount of lights in the scene
    
    static Lights empty() {
        Lights l{};

        for (int i = 0; i < 20; i++) {
            l.position[i].v = glm::vec4(0.0f);
            l.color[i].v = glm::vec4(1.0f);
        }

        l.count = 0;
        return l;
    }
};
