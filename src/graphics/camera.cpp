#include "camera.h"
#include "transform.h"

/*
Frustum frustum_from_camera(
    const Camera& cam, 
    float aspect, 
    float fovY, 
    float zNear,
    float zFar
) {
    Frustum frustum;

    const float halfVSide = zFar * tanf(fovY * .5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * cam.front;

    frustum.near = { 
        cam.position + zNear * cam.front, 
        cam.front
    };
    frustum.far = { 
        cam.position + frontMultFar, 
        -cam.front 
    };
    frustum.right = { 
        cam.position, 
        glm::cross(frontMultFar - cam.right * halfHSide, cam.up) 
    };
    frustum.left = { 
        cam.position, 
        glm::cross(cam.up,frontMultFar + cam.right * halfHSide) 
    };
    frustum.top = { 
        cam.position, 
        glm::cross(cam.right, frontMultFar - cam.up * halfVSide) 
    };
    frustum.bottom = { 
        cam.Position, 
        glm::cross(frontMultFar + cam.up * halfVSide, cam.right)
    };

    return frustum;
}

bool is_on_frustum(const Frustum& frustum, const Transform& transform) {
    return true;
};
*/

void Camera::update() {
    if (pitch > 83.9) 
        pitch = 83.9;

    if (pitch < -83.9) 
        pitch = -83.9;

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

    // if (tick() % n_frames == 0)

}

// TODO: omg please find a better name for this... camera panning? look_around? i dunno just come up with something 💀💀💀
void Camera::mouse_view(bool lock_cursor, float dx, float dy, float sensitivity){
    if (!lock_cursor) {
        yaw += dx * sensitivity;
        pitch += -dy * sensitivity;
    }
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