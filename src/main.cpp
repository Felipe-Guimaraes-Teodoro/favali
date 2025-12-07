#include <stdio.h>
#include <vector>
#include <memory>
using std::vector;
#include <math.h>
#include "SDL3/SDL.h"
#include "scripting.h"

#include "glad/glad.h"

#include "transform.h"
#define UBO_DEFINITION
#include "shaders.h"
#include "mesh.h"
#include "shapes.h"
#include "camera.h"
#include "light.h"
#include "texture.h"
#include "model.hpp"
#include "level.h"
#include "sod.h"

#include "player.h"
#include "bullet.h"
#include "BVH.h"
#include "geometry.h"
#include "raycast.h"

#define CLEAR_SCREEN glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

SDL_Window* window;
SDL_GLContext gl_context;

int w = 1600 * 0.6;
int h = 900 * 0.6;

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindow("OpenGL!", w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_SetWindowRelativeMouseMode(window, true);
    gl_context = SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("could not initialize glad");
    }
    SDL_GL_MakeCurrent(window, gl_context);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glViewport(0, 0, w, h);
    glClearColor(0.1, 0.2, 0.3, 1.0);
}

void onResize(int w, int h, Camera& camera) {

    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    camera.proj = glm::perspective(
        glm::radians(80.f),
        aspect,
        0.1f,
        1000.f
    );
}

void cameraMovement(bool lock_cursor, vec3& goal, Camera& camera, float sensitivity, float speed, float dt){
    static SOD cam_sod = create_sod(1.5, 0.5, 0.5, goal);
    
    const bool *state = SDL_GetKeyboardState(NULL);

    float mouse_x, mouse_y;
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
    
    if (!lock_cursor) {
        camera.yaw += mouse_x * sensitivity;
        camera.pitch += -mouse_y * sensitivity;
    }

    if (state[SDL_SCANCODE_W]){
        camera.position += camera.front * dt * speed;
    }
    if (state[SDL_SCANCODE_S]){
        camera.position -= camera.front * dt * speed;
    }
    if (state[SDL_SCANCODE_A]){
        camera.position += camera.right * dt * speed;
    }
    if (state[SDL_SCANCODE_D]){
        camera.position -= camera.right * dt * speed;
    }
    if (state[SDL_SCANCODE_SPACE]){
        camera.position.y += dt * speed;
    }
    if (state[SDL_SCANCODE_LCTRL]){
        camera.position.y -= dt * speed;
    }

    update_sod(cam_sod, dt, goal);

    camera.position = cam_sod.y;
    camera.update();
}

int main() {
    init();

    SDL_Event event;

    Shader vs = create_shader(&default_vs, GL_VERTEX_SHADER);
    Shader fs = create_shader(&default_fs, GL_FRAGMENT_SHADER);    
    unsigned int program = create_program(vs, fs);

    Shape gun = create_shape_from_gltf("../../../assets/gun.gltf", 0);
    glm::vec3 local_shoot_pos(3.2, -5.97, 0.);
    gun.transform.scale = vec3(0.01);
    std::vector<Bullet> bullets;

    Level *level0 = create_level_from_gltf("../../../assets/level0.glb");
    std::vector<BVHNode*> worldBVHs;
    for (int i = 0; i < level0->shapes.size(); i++){
        std::vector<MeshTriangle> world_tris = get_level_tris(&level0->shapes[i]);
        worldBVHs.push_back(buildBVH(world_tris));
    }
    
    Camera camera = create_camera({0, 0, 0}, 80.0, 1600.0 / 900.0);
    float playerRadius = 1.0;

    Light light = Light::empty();
    light.position = {1.0 ,10.0, 5.0};
    light.color = {1.0, 1.0, 1.0};

    UniformBuffer ubo = create_ubo<Light>(&light);
    bind_ubo("Light", 0, ubo, fs);

    Uint64 last = SDL_GetTicks();

    float fps = 60.0f;
    float dt = 0.0f;
    float speed = 5.5f;
    float sensitivity = 0.1;
    bool shoot = false;
    static float recoil_timer = 0.f;
    vec3 camera_goal = camera.position;

    bool lock_cursor = false;
    bool running = true;
    while (running) {
        Uint64 now = SDL_GetTicks();
        dt = (now - last)/1000.0f;
        last = now;

        light.position = camera.position + vec3(0.0, 15.0, 0.);
        update_ubo<Light>(ubo, &light);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            if (event.type == SDL_EVENT_KEY_DOWN){
                if (event.key.key == SDLK_F){
                    exec_script();
                    bullets.clear();
                }

                if (event.key.key == SDLK_LALT){
                    if (lock_cursor){
                        SDL_SetWindowRelativeMouseMode(window, lock_cursor);
                        SDL_HideCursor();
                        lock_cursor = false;
                    }
                    else{
                        SDL_SetWindowRelativeMouseMode(window, lock_cursor);
                        SDL_ShowCursor();
                        lock_cursor = true;
                    }
                }
                if (event.key.key == SDLK_ESCAPE){
                    running = false;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                if (event.button.button == SDL_BUTTON_LEFT){
                    glm::vec3 muzzleWorld = glm::vec3(
                        gun.transform.getModelMat() * glm::vec4(local_shoot_pos, 1.0)
                    );
                    glm::vec3 forward = glm::normalize(glm::vec3(gun.transform.getModelMat() * glm::vec4(1,0,0,0)));

                    bullets.emplace_back(create_bullet(muzzleWorld, forward));
                    shoot = true;
                }
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);

                onResize(w, h, camera);
            }
        }
        
        cameraMovement(lock_cursor, camera_goal, camera, sensitivity, speed, dt);

        AABB query;
        query.min = camera_goal - glm::vec3(playerRadius);
        query.max = camera_goal + glm::vec3(playerRadius); // create bounding box roughly the size of the player

        std::vector<MeshTriangle> candidates;
        for (int i = 0; i < worldBVHs.size(); i++){
            bvhQuery(worldBVHs[i], query, candidates); // get possible triangles to intersect
    
            for (auto& tri : candidates){
                // closest tri to point
                glm::vec3 closest = closestPointOnTriangle(camera_goal, tri);
    
                glm::vec3 triNormal = tri.getTriangleNormal();

                glm::vec3 diff = camera_goal - closest;
                float dist = length(diff);

                if (dist < playerRadius) {
                    float push = playerRadius - dist;

                    // empurra pela normal do tri, nao pelo diff
                    camera_goal += triNormal * push;
                }
            }
            candidates.clear();
        }
        
        CLEAR_SCREEN;
    
        gun.draw(program, camera);

        for (const Shape& shape : level0->shapes) {
            shape.draw(program, camera);
        }

        for (int i = 0; i < bullets.size(); i++){
            bullets[i].shape->draw(program, camera);
            bullets[i].shape->transform.position += bullets[i].dir * dt * 1.f;
        }
        
        if (shoot) {
            recoil_timer += 1.f;
            recoil_timer = glm::min(recoil_timer, 1.5f);
            shoot = false;
        }

        vec3 goal = camera.position + camera.front*0.3f + -camera.right * 0.12f - camera.up * 0.1f;
        goal -= camera.front * (recoil_timer * 0.04f);

        recoil_timer = glm::max(0.f, recoil_timer - dt * 6.f);

        float recoil_amount = recoil_timer * -0.3f;

        glm::quat base = glm::quatLookAt(-camera.right, -camera.up);
        glm::quat recoil_q = glm::angleAxis(recoil_amount, camera.right);

        glm::quat target_rot = recoil_q * base;

        gun.transform.rotation = glm::slerp(gun.transform.rotation, target_rot, 0.15f);
        gun.transform.position = goal;

        for (int i = 0; i < bullets.size(); i++){
            float speed = 50.0f;
            
            // Ray r;
            // r.origin = bullets[i].shape.transform.position;
            // r.direction = normalize(bullets[i].dir);
            
            bullets[i].shape->draw(program, camera);
            bullets[i].update_bullet(speed, dt);
            
            // r.tMax = dt;

            // AABB rayBox = makeAABB_from_ray(r);
            // std::vector<MeshTriangle> candidates;
            // bvhQuery(worldBVH, rayBox, candidates);

            // float closestT = 999999.0f;
            // bool hit = false;
            // for(auto& tri : candidates){
            //     float t;
            //     if (r.hitTriangle(tri, t)){
            //         if (t < closestT){
            //             closestT = t;
            //             hit = true;
            //             break;
            //         }
            //     }
            // }

            // if (hit){
            //     SDL_Log("Eita lasquerrrr bateu!!!");
            // }
        }

        SDL_GL_SwapWindow(window);

        float target = 1000.0f / fps;
        float frame_ms = dt * 1000.0f;

        if (frame_ms < target) {
            SDL_Delay((Uint32)(target - frame_ms));
        }
    }

    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(gl_context);
    SDL_Quit();
    scripting_quit();

    return 0;
}
