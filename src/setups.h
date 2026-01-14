#pragma once 

#include "SDL3/SDL.h"
#include "glad/glad.h"
#include "audio.h"
#include "ui.h"
#include "gizmos.h"
#include "scripting.h"

#define CLEAR_SCREEN glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)


void on_resize(int w, int h, Camera& camera) {
    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    camera.aspect = aspect;

    camera.proj = glm::perspective(
        glm::radians(camera.fov_y),
        camera.aspect,
        camera.z_near,
        camera.z_far
    );
}


void init(SDL_Window** window, SDL_GLContext *gl_context, int width, int height) {
    // sdl and gl
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    *window = SDL_CreateWindow("FAVALI", width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_SetWindowRelativeMouseMode(*window, true);
    *gl_context = SDL_GL_CreateContext(*window);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("could not initialize glad");
    }
    SDL_GL_MakeCurrent(*window, *gl_context);

    init_audio();

    // imgui
    imgui_init(*window, *gl_context);
    
    // gl, our renderer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
        
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glViewport(0, 0, width, height);
    glClearColor(0.1, 0.2, 0.3, 1.0);

    init_gizmos();
}

void end_process(SDL_Window** window, SDL_GLContext *gl_context){
    SDL_DestroyWindow(*window);
    SDL_GL_DestroyContext(*gl_context);
    SDL_Quit();
    imgui_shutdown();
    scripting_quit();
    audio_quit();
}
