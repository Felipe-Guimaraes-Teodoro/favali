#include "camera.h"

void Camera::update() {
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

Camera create_camera(
    vec3 pos,
    float fov,
    float aspect
) {
    Camera cam = {};

    cam.yaw = -90.0f;

    cam.proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
    cam.front = {0.0, 0.0, -1.0};
    cam.position = pos;

    cam.update();

    return cam;
}