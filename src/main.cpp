#include <stdio.h>
#include <vector>
using std::vector;
#include "SDL3/SDL.h"
#include "scripting.h"

#include "glad/glad.h"

#include "transform.h"
#include "shaders.h"
#include "mesh.h"
#include "shapes.h"
#include "camera.h"
#include "light.h"

SDL_Window* window = NULL;
SDL_GLContext gl_context;

int w = 900;
int h = 600;

void clearWindow(); // combiler: bro why did u even bother

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindow("todo: name this", w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_SetWindowRelativeMouseMode(window, true);
    gl_context = SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("could not initialize glad");
    }
    SDL_GL_MakeCurrent(window, gl_context);
    
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, w, h);
    glClearColor(0.1, 0.2, 0.3, 1.0);
}

void onResize(int w, int h, Camera& camera) {

    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    camera.proj = glm::perspective(
        glm::radians(60.f),
        aspect,
        0.1f,
        100.f
    );
}

int main() {
    init();

    SDL_Event event;

    Shader vs = create_shader(&default_vs, GL_VERTEX_SHADER);
    Shader fs = create_shader(&default_fs, GL_FRAGMENT_SHADER);    
    unsigned int program = create_program(vs, fs);

    // this kinda rolls well off the tongue
    vector<Shape> squares = {};

    Camera camera = create_camera({0, 0, 0}, 60.0);

    Light light = Light::empty();
    light.position = {1.0 ,1.0, 1.0};
    light.color = {1.0, 1.0, 1.0};

    UniformBuffer ubo = create_ubo<Light>(&light);
    bind_ubo("Light", 0, ubo, fs);

    Uint64 last = SDL_GetTicks();
    float fps = 60.0f;
    float dt = 0.0f;
    float speed = 5.5f;
    float sensitivity = 0.1;

    bool lock_cursor = false;
    bool running = true;
    while (running) {
        Uint64 now = SDL_GetTicks();
        dt = (now - last)/1000.0f;
        last = now;

        light.position = camera.position;
        
        update_ubo<Light>(ubo, &light);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            if (event.type == SDL_EVENT_KEY_DOWN){
                if (event.key.key == SDLK_F){
                    exec_script();
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
                if (event.key.key == SDLK_C){
                    for (int i = 0; i < 50; i++) {
                        Shape shape = make_shape(Shapes::Cube);
                        shape.transform.position = camera.position;
                        squares.push_back(shape);
                    }
                }

                if (event.key.key == SDLK_R) {
                    for (int i = 0; i < 50; i++) {
                        // todo: fix memory leak
                        squares.erase(squares.begin() + i);
                    }
                }
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);

                onResize(w, h, camera);
            }
        }

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
        if (state[SDL_SCANCODE_E]){
            camera.position.y += dt * speed;
        }
        if (state[SDL_SCANCODE_Q]){
            camera.position.y -= dt * speed;
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        for (const Shape& square : squares) {
            square.draw(program, camera);
        }

        camera.update();

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
