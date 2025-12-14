#include "camera.h"
#include "transform.h"

inline float aabb_distance_from_plane(const Plane& plane, const AABB& aabb) {
    glm::vec3 c = (aabb.min + aabb.max) * 0.5f; // center
    glm::vec3 e = (aabb.max - aabb.min) * 0.5f; // positive extents

    float r = e.x * fabs(plane.normal.x)
            + e.y * fabs(plane.normal.y)
            + e.z * fabs(plane.normal.z);

    float s = glm::dot(plane.normal, c) - plane.distance;

    return s - r;
}

bool is_aabb_on_frustum(const Frustum& frustum, const AABB& aabb) {
    if (aabb_distance_from_plane(frustum.near, aabb) < 0) return false;
    if (aabb_distance_from_plane(frustum.far, aabb) < 0) return false;
    if (aabb_distance_from_plane(frustum.left, aabb) < 0) return false;
    if (aabb_distance_from_plane(frustum.right, aabb) < 0) return false;
    if (aabb_distance_from_plane(frustum.top, aabb) < 0) return false;
    if (aabb_distance_from_plane(frustum.bottom, aabb) < 0) return false;

    return true;
}
// utility to create a plane from three points
inline Plane makePlane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
    float distance = glm::dot(normal, a);
    return { normal, distance };
}

void Camera::update_frustum() {
    glm::vec3 nc = position + front * z_near;
    glm::vec3 fc = position + front * z_far;

    float nearHeight = 2.0f * z_near * tanf(fov_y * 0.5f);
    float nearWidth  = nearHeight * aspect;
    float farHeight  = 2.0f * z_far * tanf(fov_y * 0.5f);
    float farWidth   = farHeight * aspect;

    glm::vec3 upN = up * (nearHeight * 0.5f);
    glm::vec3 rightN = right * (nearWidth * 0.5f);

    glm::vec3 ntl = nc + upN - rightN;
    glm::vec3 ntr = nc + upN + rightN;
    glm::vec3 nbl = nc - upN - rightN;
    glm::vec3 nbr = nc - upN + rightN;

    glm::vec3 upF = up * (farHeight * 0.5f);
    glm::vec3 rightF = right * (farWidth * 0.5f);

    glm::vec3 ftl = fc + upF - rightF;
    glm::vec3 ftr = fc + upF + rightF;
    glm::vec3 fbl = fc - upF - rightF;
    glm::vec3 fbr = fc - upF + rightF;

    frustum.near = makePlane(ntr, ntl, nbl);
    frustum.far = makePlane(ftl, ftr, fbr);
    frustum.left = makePlane(ntl, ftl, fbl);
    frustum.right = makePlane(ftr, ntr, nbr);
    frustum.top = makePlane(ntl, ntr, ftr);
    frustum.bottom = makePlane(nbr, nbl, fbl);
}


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
        // create frustum, cull
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
    cam.fov_y = fov;
    cam.z_near = 0.1;
    cam.z_far = 1000.0;
    cam.aspect = aspect;

    cam.proj = glm::perspective(glm::radians(cam.fov_y), cam.aspect, cam.z_near, cam.z_far);
    cam.front = {0.0, 0.0, -1.0};
    cam.position = pos;

    cam.update();

    return cam;
}