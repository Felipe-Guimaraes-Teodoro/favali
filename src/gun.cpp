#include "gun.h"
#include "camera.h"
#include "glm.hpp"
#include <vector>
#include <deque>

void Gun::update(float dt, Camera& camera){
    last_shot += dt;
    
    vec3 goal = camera.position + camera.front*0.3f + -camera.right * 0.12f - camera.up * 0.1f;
    goal -= camera.front * (recoil_timer * 0.04f);

    recoil_timer = glm::max(0.f, recoil_timer - dt * 6.f);

    float recoil_amount = recoil_timer * -0.3f;

    glm::quat base = glm::quatLookAt(-camera.right, -camera.up);
    glm::quat recoil_q = glm::angleAxis(recoil_amount, camera.right);

    glm::quat target_rot = recoil_q * base;

    shape->transform.rotation = glm::slerp(shape->transform.rotation, target_rot, 0.15f);
    shape->transform.position = goal;
    
    if (last_shot >= cooldown && is_shooting){
        last_shot = 0.0f;
        glm::vec3 dir = glm::normalize(glm::vec3(shape->transform.getModelMat() * glm::vec4(1,0,0,0)));
        glm::vec3 muzzle_world = glm::vec3(
            shape->transform.getModelMat() * glm::vec4(muzzle_pos, 1.0)
        );

        recoil_timer += 1.f;
        recoil_timer = glm::min(recoil_timer, 1.5f);

        
        for (int i = 0; i < bullets_per_shot; i++){
            Bullet b = create_bullet(muzzle_world, dir);
            b.damage = bullet_template.damage;
            b.lifetime = bullet_template.lifetime;
            b.speed = bullet_template.speed;

            bullets.emplace_back(std::move(b));
        }
    }

    for (int i = 0; i < bullets.size(); i++){
        bullets[i].update_bullet(dt);
    }
}

void Gun::draw(unsigned int program, Camera& camera){
    shape->draw(program, camera);

    std::deque<int> dead_bullets;

    for (int i = 0; i < bullets.size(); i++){
        if (bullets[i].lifetime > 0.){
            bullets[i].shape->draw(program, camera);
        } else{
            dead_bullets.push_back(i);
        }
    }

    for (int k = dead_bullets.size() - 1; k >= 0; k--){ // go backwards so theres no index mishap
        int idx = dead_bullets[k];
        bullets.erase(bullets.begin() + idx);
        dead_bullets.pop_front();
    }
}

Gun make_gun(glm::vec3 muzzle_pos, float cooldown, float spread, unsigned int bullets_per_shot){
    Gun g;
    g.recoil_timer = 0.;
    g.muzzle_pos = muzzle_pos;
    g.cooldown = cooldown;
    g.spread = spread;
    g.bullets_per_shot = bullets_per_shot;
    g.is_shooting = false;

    return g;
}
