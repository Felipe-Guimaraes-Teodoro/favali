#include "bullet.h"

void Bullet::update_bullet(float speed, float dt){
    shape.transform.position += dir * speed * dt;
}
