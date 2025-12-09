#include "bullet.h"

void Bullet::update_bullet(float dt){
    if (lifetime <= 0.){
        return;
    }
    shape->transform.position += dir * speed * dt;
    lifetime -= dt;
}

Bullet create_bullet(const glm::vec3& muzzle_world, const glm::vec3& forward) {
    Bullet b;
    b.shape = std::make_unique<Shape>(make_shape(Shapes::Sphere));
    b.shape->transform.position = muzzle_world;
    b.shape->transform.scale = glm::vec3(0.05f);

    b.dir = forward;
    BulletTemplate bt = create_bullet_template();
    b.lifetime = bt.lifetime;
    b.damage = bt.damage;
    b.speed = bt.speed;
    return b;
}

BulletTemplate create_bullet_template() {
    BulletTemplate b;
    b.lifetime = 5.;
    b.damage = 1.;
    b.speed = 50.0f;
    return b;
}
