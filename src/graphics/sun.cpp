#include "sun.h"
#include "gtc/matrix_transform.hpp"
#include "shaders.h"
#include "level.h"
#include <vector>

void Sun::updateSun(float time){
    float t = time * 0.05f;
    
    sunDir = glm::normalize(glm::vec3(
        cos(t),
        sin(t),
        0.2f
    ));

    glm::vec3 sunPos = sunDir*200.0f;
    sunView = glm::lookAt(
        sunPos,
        glm::vec3(0.),
        glm::vec3(0., 1., 0.)
    );

    float near_plane = 1.0f;
    float far_plane  = 400.0f;

    sunProj = glm::ortho(
        -100.0f, 100.0f,
        -100.0f, 100.0f,
        near_plane,
        far_plane
    );

    sunSpaceMatrix = sunProj * sunView;
}

void Sun::renderShadow(unsigned int depthProgram, const std::vector<Level*>& levels, int screenW, int screenH, int shadowSize){
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
    
    shader_uniform_mat4(depthProgram, "lightSpaceMatrix", sunSpaceMatrix);

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

Sun createSun(int shadowSize)
{
    Sun sun{};

    sun.sunDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));

    glGenTextures(1, &sun.depthMap);
    glBindTexture(GL_TEXTURE_2D, sun.depthMap);

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

    glGenFramebuffers(1, &sun.depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sun.depthFBO);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        sun.depthMap,
        0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    sun.sunSpaceMatrix = glm::mat4(1.0f);
    return sun;
}
