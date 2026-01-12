#pragma once

typedef struct {
    glm::vec3 v;
    float padding;
} PaddedVec3;

struct Lights {
    PaddedVec3 position[20]; // 20 is the default maximum allowed lights
    PaddedVec3 color[20];
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
