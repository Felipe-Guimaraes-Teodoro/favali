#pragma once

struct Light{
    glm::vec3 position;
    float padding1;
    glm::vec3 color;
    float padding2;
    
    static Light empty(){
        return Light{
            glm::vec3(0.),
            0.0,
            glm::vec3(1.),
            0.0
        };
    }
    
};
