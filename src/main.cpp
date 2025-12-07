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

SDL_Window* window;
SDL_GLContext gl_context;

int w = 1600;
int h = 900;

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

void cameraMovement(bool lock_cursor, Camera& camera, SOD& camera_sod, float sensitivity, float speed, float dt){
    static vec3 goal = {0.0, 0.0, 0.0};
    const bool *state = SDL_GetKeyboardState(NULL);

    float mouse_x, mouse_y;
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
    
    if (!lock_cursor) {
        camera.yaw += mouse_x * sensitivity;
        camera.pitch += -mouse_y * sensitivity;
    }

    if (state[SDL_SCANCODE_W]){
        goal += camera.front * dt * speed;
    }
    if (state[SDL_SCANCODE_S]){
        goal -= camera.front * dt * speed;
    }
    if (state[SDL_SCANCODE_A]){
        goal += camera.right * dt * speed;
    }
    if (state[SDL_SCANCODE_D]){
        goal -= camera.right * dt * speed;
    }
    if (state[SDL_SCANCODE_SPACE]){
        goal.y += dt * speed;
    }
    if (state[SDL_SCANCODE_LCTRL]){
        goal.y -= dt * speed;
    }

    update_sod(camera_sod, dt, goal);

    camera.position = camera_sod.y;
}


typedef struct {
    std::unique_ptr<Shape> bullet;
    glm::vec3 dir;
} Bullet;

Bullet create_bullet(const glm::vec3& muzzleWorld, const glm::vec3& forward) {
    Bullet b;
    b.bullet = std::make_unique<Shape>(make_shape(Shapes::Sphere));
    b.bullet->transform.position = muzzleWorld;
    b.bullet->transform.scale = glm::vec3(0.05f);
    b.dir = forward;
    return b;
}

int main() {
    init();

    SDL_Event event;

    Shader vs = create_shader(&default_vs, GL_VERTEX_SHADER);
    Shader fs = create_shader(&default_fs, GL_FRAGMENT_SHADER);    
    unsigned int program = create_program(vs, fs);

    Shape gun = create_shape_from_gltf("../../../assets/gun.gltf", 0);
    glm::vec3 local_shoot_pos(-3.2, -5.97, 0.);
    std::vector<Bullet> bullets;

    Level *level0 = create_level_from_gltf("../../../assets/level1.glb");
    gun.transform.scale = vec3(0.01);
    
    SOD cam_sod = create_sod(1.5, 0.5, 0.5, vec3(0.0f));
    Camera camera = create_camera({0, 0, 0}, 80.0, (float)w/(float)h);

    Light light = Light::empty();
    light.position = {1.0 ,10.0, 5.0};
    light.color = {1.0, 1.0, 1.0};

    UniformBuffer ubo = create_ubo<Light>(&light);
    bind_ubo("Light", 0, ubo, fs);

    Uint64 last = SDL_GetTicks();

    // todo: add bullet stuff into its separate thingy
    float fps = 60.0f;
    float dt = 0.0f;
    float speed = 5.5f;
    float sensitivity = 0.1;
    bool shoot = false;
    static float recoil_timer = 0.f;

    bool lock_cursor = false;
    bool running = true;
    while (running) {
        Uint64 now = SDL_GetTicks();
        dt = (now - last)/1000.0f;
        last = now;

        // light.position = camera.position + vec3(5.0, 5.0, 5.);
        // update_ubo<Light>(ubo, &light);

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
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        gun.draw(program, camera);

        for (const Shape& shape : level0->shapes) {
            shape.draw(program, camera);
        }

        for (int i = 0; i < bullets.size(); i++){
            bullets[i].bullet->draw(program, camera);
            bullets[i].bullet->transform.position += bullets[i].dir * dt * 1.f;
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
        gun.transform.position = glm::mix(gun.transform.position, goal, 0.5f);

        camera.update();
        cameraMovement(lock_cursor, camera, cam_sod, sensitivity, speed, dt);

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
