#include "bullet.h"

void Bullet::update_bullet(float speed, float dt){
    shape->transform.position += dir * speed * dt;
}

Bullet create_bullet(const glm::vec3& muzzleWorld, const glm::vec3& forward) {
    Bullet b;
    b.shape = std::make_unique<Shape>(make_shape(Shapes::Sphere));
    b.shape->transform.position = muzzleWorld;
    b.shape->transform.scale = glm::vec3(0.05f);
    b.dir = forward;
    return b;
}
