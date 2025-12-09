#include "bullet.h"
#include "raycast.h"
#include "BVH.h"
#include "player.h"
#include <stdio.h>

void Bullet::update_bullet(float dt){
    if (lifetime <= 0.){
        return;
    }
    shape->transform.position += (dir * speed + GRAVITY) * dt;
    lifetime -= dt;
}

bool Bullet::handle_collisions(float dt, std::vector<BVHNode*> worldBVHs /* add enemy AABBs in the future here too */){
    Ray r;
    r.origin = shape->transform.position;
    r.direction = dir;
    
    r.tMax = dt;

    AABB rayBox = makeAABB_from_ray(r);
    std::vector<MeshTriangle> candidates;

    for (int i = 0; i < worldBVHs.size(); i++){
        bvhQuery(worldBVHs[i], rayBox, candidates);
        
        float closestT = 999999.0f;
        for(auto& tri : candidates){
            float t;
            if (r.hitTriangle(tri, t)){
                if (t < closestT){
                    closestT = t;
                    return true; // move return out of here so we can get the precise location of impact
                }
            }
        }
        
        candidates.clear();
    }

    return false;
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
