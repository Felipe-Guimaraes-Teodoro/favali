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

    void update() {
        if (pitch > 89.9) 
            pitch = 89.9;

        if (pitch < -89.9) 
            pitch = -89.9;

        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(front);

        right = glm::normalize(glm::cross(UP, front));
        up = glm::cross(front, right);

        view = glm::lookAt(
            position,
            position + front,
            UP
        );
    }
} Camera;

Camera create_camera(
    vec3 pos = {0.0f, 0.0f, 1.0f},
    float fov = 80.0f,
    float aspect = 900.0f / 600.0f
) {
    Camera cam = {};

    cam.yaw = -90.0f;

    cam.proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
    cam.front = {0.0, 0.0, -1.0};
    cam.position = pos;

    cam.update();

    return cam;
}