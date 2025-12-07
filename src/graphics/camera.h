#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

using glm::mat4;
using glm::vec3;

static vec3 UP = {0.0, 1.0, 0.0};

typedef struct {
    mat4 proj;
    mat4 view;

    vec3 position;

    vec3 front;
    vec3 right;
    vec3 up;

    float roll, yaw, pitch;

    void update();
} Camera;

Camera create_camera(
    vec3 pos = {0.0f, 0.0f, 1.0f},
    float fov = 80.0f,
    float aspect = 900.0f / 600.0f
);