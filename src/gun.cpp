#include <vector>
#include <deque>
#include <random>

#include "gun.h"
#include "camera.h"
#include "glm.hpp"
#include "ik.h"
#include "audio.h"

void Gun::update(float dt, Camera& camera, IkController& controller, Player& player, std::vector<BVHNode*> worldBVHs){
    last_shot += dt;
    
    vec3 goal = glm::mix(camera.position, player.position + player.head_ofs, -0.25f) + camera.front*0.3f + -camera.right * 0.12f - camera.up * 0.1f;
    goal -= camera.front * (recoil_timer * 0.04f);
    recoil_timer = glm::max(0.f, recoil_timer - dt * 6.f);

    float recoil_amount = recoil_timer * -0.3f;
    controller.goal = goal + camera.front * 0.3f - camera.up * 0.2f * camera.right * 0.6f;

    glm::quat base = glm::quatLookAt(-camera.right, -camera.up);
    glm::quat recoil_q = glm::angleAxis(recoil_amount, camera.right);

    glm::quat target_rot = recoil_q * base;

    shape->transform.rotation = glm::slerp(shape->transform.rotation, target_rot, 0.3f);
    shape->transform.position = controller.leaf->end + camera.front * 0.1f + camera.up * 0.1f;
    
    if (last_shot >= cooldown && is_shooting){
        last_shot = 0.0f;

        recoil_timer += 1.f;
        recoil_timer = glm::min(recoil_timer, 1.5f);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-spread, spread);

        glm::vec3 muzzle_world = glm::vec3(shape->transform.getModelMat() * glm::vec4(muzzle_pos, 1.0));
        
        for (int i = 0; i < bullets_per_shot; i++){
            float x = dist(gen);
            float y = dist(gen);
            float z = dist(gen);

            glm::vec3 rand_spread(x, y, z);
            rand_spread *= 0.2;

            glm::vec4 dir = glm::vec4(muzzle_pos - muzzle_back_sample, 0.);
            dir -= glm::vec4(rand_spread, 0.);
            glm::vec3 dir_norm = glm::normalize(glm::vec3(shape->transform.getModelMat() * dir));

            Bullet b = create_bullet(muzzle_world, dir_norm);
            b.damage = bullet_template.damage;
            b.lifetime = bullet_template.lifetime;
            b.speed = bullet_template.speed;

            bullets.emplace_back(std::move(b));
        }

        if (audio_ctx->samples[0]) {
            audio_ctx->samples[0]->cursor = 0;
            audio_ctx->samples[0]->is_playing = true;
        } else {
            audio_ctx->samples[0] = load_wav("assets/bullet.wav", 0.1, false);
            // audio_ctx->samples[0]->pan = 1.0;
        }
    }

    for (int i = 0; i < bullets.size(); i++){
        if (bullets[i].handle_collisions(dt, worldBVHs)){
            float dot = glm::dot(bullets[i].dir, bullets[i].normal);

            if (dot > -0.5) {
                bullets[i].speed *= 0.5f;
                // reflection
                bullets[i].dir = glm::normalize(
                    bullets[i].dir - 2.0f * dot * bullets[i].normal
                );

                if (audio_ctx->samples[1]) {
                    audio_ctx->samples[1]->cursor = 500 * sizeof(float) * 2;
                    audio_ctx->samples[1]->emitter.pos = bullets[i].shape->transform.position;
                    audio_ctx->samples[1]->is_playing = true;
                } else {
                    audio_ctx->samples[1] = load_wav("assets/bullet.wav", 0.1, false);
                    audio_ctx->samples[1]->emitter.not_positioned = false;
                    audio_ctx->samples[1]->emitter.pos = bullets[i].shape->transform.position;
                    audio_ctx->samples[1]->pitch = 1.5;
                    // audio_ctx->samples[0]->pan = 1.0;
                }
            } else {
                bullets[i].lifetime = 0.0f;

                int r = (rand() % 1000) - 500;
            
                Transform t = Transform::empty();
                t.position = bullets[i].shape->transform.position;
                t.position += bullets[i].normal*0.01f;
                t.rotation = glm::rotation(glm::vec3(0,0,-1), bullets[i].normal)
                    * glm::angleAxis(r / 500.0f, vec3(0, 0, -1));
                t.scale *= 0.2 + r / 5000.0f;

                push_instance(bullet_hole_mesh, t.getModelMat());
                update_instances(bullet_hole_mesh);

                if (audio_ctx->samples[1]) {
                    audio_ctx->samples[1]->cursor = 500 * sizeof(float) * 2;
                    audio_ctx->samples[1]->emitter.pos = bullets[i].shape->transform.position;
                    audio_ctx->samples[1]->is_playing = true;
                } else {
                    audio_ctx->samples[1] = load_wav("assets/bullet.wav", 0.1, false);
                    audio_ctx->samples[1]->emitter.not_positioned = false;
                    audio_ctx->samples[1]->emitter.pos = bullets[i].shape->transform.position;
                    audio_ctx->samples[1]->pitch = 0.5;
                    // audio_ctx->samples[0]->pan = 1.0;
                }
            }

        }
        bullets[i].update_bullet(dt);
    }
}

void Gun::draw(unsigned int program, unsigned int instanced_program, Camera& camera){
    shape->draw(program, camera);

    std::deque<int> dead_bullets;

    for (int i = 0; i < bullets.size(); i++){
        if (bullets[i].lifetime > 0.){
            bullets[i].shape->draw(program, camera);
        } else{
            dead_bullets.push_back(i);
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // prevents Z-fighting
    bullet_hole_mesh.draw(instanced_program, camera.view, camera.proj, glm::vec4(1.), bullet_hole_tex);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    for (int k = dead_bullets.size() - 1; k >= 0; k--) {
        int idx = dead_bullets[k];
        bullets.erase(bullets.begin() + idx);
    }
    dead_bullets.clear();
}

Gun make_gun(glm::vec3 muzzle_back_sample, glm::vec3 muzzle_pos, float cooldown, float spread, unsigned int bullets_per_shot, unsigned int bullet_hole_tex){
    Gun g;
    g.recoil_timer = 0.;
    g.muzzle_back_sample = muzzle_back_sample;
    g.muzzle_pos = muzzle_pos;
    g.cooldown = cooldown;
    g.spread = spread;
    g.bullets_per_shot = bullets_per_shot;
    g.last_shot = 0.0;
    g.is_shooting = false;
    
    g.bullet_hole_tex = bullet_hole_tex;
    Shape bs = make_shape(Shapes::Square, bullet_hole_tex); // Bullet Shape :)
    g.bullet_hole_mesh = mesh_to_instanced(bs.mesh);

    return g;
}
