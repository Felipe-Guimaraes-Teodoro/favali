#include "spot_light.h"
#include "gtc/matrix_transform.hpp"
#include "shaders.h"
#include "level.h"
#include <vector>

void SpotLight::updateLight(glm::vec3 lightPos){
    view = glm::lookAt(
        lightPos,
        glm::vec3(0.),
        glm::vec3(0., 1., 0.)
    );

    float near_plane = 1.0f;
    float far_plane  = 30.0f;

    proj = glm::perspective(
        glm::radians(fov),
        1.0f,
        near_plane,
        far_plane
    );

    lightSpaceMatrix = proj * view;
}

void SpotLight::renderShadow(unsigned int depthProgram, const std::vector<Level*>& levels, int screenW, int screenH, int shadowSize){
    if (!glIsProgram(depthProgram)) {
        printf("DEPTH PROGRAM INVALID: %u\n", depthProgram);
        return;
    }

    glUseProgram(depthProgram);
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glViewport(0, 0, shadowSize, shadowSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glCullFace(GL_FRONT);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    
    shader_uniform_mat4(depthProgram, "lightSpaceMatrix", lightSpaceMatrix);

    for (Level* lvl : levels) {
        for (auto& s : lvl->shapes) {
            s.mesh.drawDepth(depthProgram, s.transform.getModelMat());
        }
    }

    //glDepthMask(GL_FALSE);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenW, screenH);
}

SpotLight createSpotLight(int shadowSize) {
    SpotLight light{};

    light.fov = 25.0f;
    light.color = glm::vec3(1.);

    light.dir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));

    glGenTextures(1, &light.depthMap);
    glBindTexture(GL_TEXTURE_2D, light.depthMap);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        shadowSize,
        shadowSize,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glGenFramebuffers(1, &light.depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, light.depthFBO);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        light.depthMap,
        0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    light.lightSpaceMatrix = glm::mat4(1.0f);
    return light;
}
