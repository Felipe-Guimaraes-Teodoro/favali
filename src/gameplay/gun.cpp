#include <vector>
#include <deque>

#include "gun.h"
#include "camera.h"
#include "glm.hpp"
#include "audio.h"
#include "shaders.h"
#include "utils.h"

void Gun::update(float dt, Camera& camera, IkController& controller, Player& player, std::vector<BVHNode*> worldBVHs){
    last_shot += dt;

    if (aiming){
        shape->transform.position = glm::mix(shape->transform.position, camera.position + (camera.front * (0.5f + aim_offset.z)) - (camera.up * (aim_offset.y)) + (camera.right * (aim_offset.x)), 0.5);
        
        controller.goal = shape->transform.position;
    } else{
        shape->transform.position = glm::mix(shape->transform.position, camera.position + (camera.front) - (camera.up * 0.3f) - (camera.right * (0.3f)), 0.5);
        
        controller.goal = shape->transform.position;
    }

    glm::quat look = glm::quatLookAt(camera.front, -camera.up);

    glm::quat vertical_q = glm::angleAxis(glm::radians(vertical_recoil), camera.right);
    
    glm::quat lateral_q = glm::angleAxis(glm::radians(lateral_recoil), camera.up);

    glm::quat base = lateral_q * vertical_q * look;

    glm::quat fix = glm::angleAxis(glm::radians(90.f), vec3(0,1,0)); // -Z -> -X
    glm::quat final_rot = base * fix;

    shape->transform.rotation = glm::slerp(shape->transform.rotation, final_rot, 0.30f);

    // vec3 leaf_dir = glm::normalize(controller.leaf->end - controller.leaf->origin);
    // shape->transform.position = controller.leaf->end + leaf_dir * 0.1f;
    
    if (last_shot >= cooldown && is_shooting){
        last_shot = 0.0f;
        
        vertical_recoil -= randf(0.7f, 0.9f) * vertical_recoil_strength;
        lateral_recoil  += randf(-1.0f, 1.0f) * lateral_recoil_strength;
        
        std::uniform_real_distribution<float> dist(-spread, spread);
        
        glm::vec3 muzzle_world = glm::vec3(shape->transform.getModelMat() * glm::vec4(muzzle_pos, 1.0));
        glm::vec3 muzzle_back_sample = muzzle_pos - glm::vec3(0.001, 0., 0.);
        
        std::random_device rd;
        std::mt19937 gen(rd());

        for (int i = 0; i < bullets_per_shot; i++){
            glm::vec3 local_dir = glm::normalize(muzzle_pos - muzzle_back_sample);

            glm::vec3 spread_local(dist(gen), dist(gen), 0.f);
            spread_local *= spread;

            local_dir = glm::normalize(local_dir + spread_local);
            
            glm::vec3 world_dir = glm::normalize(shape->transform.rotation * local_dir);
            
            Bullet b = create_bullet(muzzle_world, world_dir);
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

    vertical_recoil = glm::min(vertical_recoil * 0.96f, 0.0f);
    lateral_recoil  *= 0.92f;

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

void Gun::draw(unsigned int program, unsigned int instanced_program, unsigned int laser_program, Camera& camera){
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

    for (int k = dead_bullets.size() - 1; k >= 0; k--) {
        int idx = dead_bullets[k];
        bullets.erase(bullets.begin() + idx);
    }
    dead_bullets.clear();

    // draw laser
    glm::vec3 begin = glm::vec3(shape->transform.getModelMat() * glm::vec4(muzzle_pos, 1.0));

    glm::vec3 forward_local = glm::vec3(1, 0, 0);

    glm::vec3 forward_world = shape->transform.rotation * forward_local;

    float length = 100.0f;

    glm::vec3 end = begin + forward_world * length;

    laser.update(begin, end, 0.05);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    laser.draw(laser_program, camera);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

}

Gun create_gun(glm::vec3 muzzle_pos, float cooldown, float spread, unsigned int bullets_per_shot, unsigned int bullet_hole_tex){
    Gun g;
    g.recoil_timer = 0.;
    g.muzzle_pos = muzzle_pos;
    g.aim_offset = glm::vec3(0.);
    g.cooldown = cooldown;
    g.spread = spread;
    g.bullets_per_shot = bullets_per_shot;
    g.last_shot = 0.0;
    g.is_shooting = false;
    g.aiming = false;

    g.vertical_recoil_strength = 10.0f;
    g.lateral_recoil_strength = 10.0f;

    g.vertical_recoil = 0.0;
    g.lateral_recoil = 0.0;
    
    g.bullet_hole_tex = bullet_hole_tex;
    Shape bs = create_shape(Shapes::Square, bullet_hole_tex); // Bullet Shape :)
    g.bullet_hole_mesh = mesh_to_instanced(bs.mesh);

    Line line = Line();

    return g;
}
