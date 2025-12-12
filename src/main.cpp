#include <stdio.h>
#include <vector>
#include <memory>
#include <math.h>
#include <arroz.brocolis🥦> // we do not know what this is, what this does, or even where it came from. But under NO CIRCUNSTANCES remove this. We. Mean. It.
#include "SDL3/SDL.h"
#include "scripting.h"

#include "glad/glad.h"
#include "ui.h"

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
#include "gun.h"
#include "gizmos.h"
#include "ik.h"
#include "audio.h"

using std::vector;

#define CLEAR_SCREEN glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

SDL_Window* window;
SDL_GLContext gl_context;

int w = 1600 * 0.5;
int h = 900 * 0.5;

void init() {
    // sdl and gl
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

    init_audio();
    // audio_ctx->smp = load_wav("assets/sound.wav");

    // imgui
    imgui_init(window, gl_context);
    
    // gl, our renderer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glViewport(0, 0, w, h);
    glClearColor(0.1, 0.2, 0.3, 1.0);

    init_gizmos();
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

int main() {
    init();

    SDL_Event event;

    Shader vs = create_shader(&default_vs, GL_VERTEX_SHADER);
    Shader fs = create_shader(&default_fs, GL_FRAGMENT_SHADER);    
    Shader vs_instanced = create_shader(&default_vs_instanced, GL_VERTEX_SHADER);
    Shader fs_instanced = create_shader(&default_fs_instanced, GL_FRAGMENT_SHADER);    
    unsigned int program = create_program(vs, fs);
    unsigned int program_instanced = create_program(vs_instanced, fs_instanced);

    std::vector<unsigned int> textures;
    textures.push_back(make_texture("../../../assets/bullet_hole.png"));

    Player player = create_player();
    audio_ctx->ud = &player;
    player.collider.radius = 1.0;

    glm::vec3 local_shoot_pos(3.2, -5.78, 0.);
    glm::vec3 muzzle_back_sample = glm::vec3(0., -5.78, 0.);
    Gun gun = make_gun(muzzle_back_sample, local_shoot_pos, 0.1f, 1., 5, textures[0]);
    gun.shape = std::make_unique<Shape>(create_shape_from_gltf("../../../assets/gun.gltf", 0));
    gun.shape->transform.scale = vec3(0.01);
    gun.bullet_template = create_bullet_template();

    SDL_Surface surface;

    Level *level0 = create_level_from_gltf("../../../assets/level2.glb");
    std::vector<BVHNode*> worldBVHs;
    for (int i = 0; i < level0->shapes.size(); i++){
        std::vector<MeshTriangle> world_tris = get_level_tris(&level0->shapes[i]);
        worldBVHs.push_back(buildBVH(world_tris));
    }
    printf("level loading complete\n");
    
    Camera camera = create_camera({0, 0, 0}, 80.0, 1600.0 / 900.0);

    Uint64 last = SDL_GetTicks();

    float fps = 60.0f;
    float dt = 0.0f;
    float sensitivity = 0.1;
    bool shoot = false;
    static float recoil_timer = 0.0f;

    bool lock_cursor = false;
    bool running = true;

    Level *arm = create_level_from_gltf("../../../assets/arm.glb");
    IkController controller = create_ik_controller({0.0, 0.0, 0.0});
    int i = 0;
    for (const Shape &shape : arm->shapes) {
        controller.push_node(create_ik_node(shape.transform.scale.y * 2.0));
        i++;
    }

    Shape tmp = make_shape(Shapes::Cube, 6);
    vector<mat4> instances = {};
    InstancedMesh i_mesh = mesh_to_instanced(tmp.mesh);

    Lights lights = level0->lights;
    UniformBuffer ubo = create_ubo<Lights>(&lights, sizeof(Lights));
    bind_ubo("Light", 0, ubo, fs);

    /*
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            glm::mat4 model = glm::mat4(1.0f);
            push_instance(
                i_mesh,
                glm::translate(model, vec3(i * 2, (rand() % 100) / 100.0, j * 2)) *
                glm::scale(model, vec3(1.6, powf((rand() % 20) / 10.0, 2), 1.6))
            );
        }
    }
    */

    update_instances(i_mesh);

    while (running) {
        Uint64 now = SDL_GetTicks();
        dt = (now - last)/1000.0f;
        last = now;

        update_ubo<void>(ubo, &lights);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            if (event.type == SDL_EVENT_KEY_DOWN){
                if (event.key.key == SDLK_F){
                    clear_instances(i_mesh);
                    update_instances(i_mesh);
                    exec_script();
                    clear_instances(gun.bullet_hole_mesh);
                    update_instances(gun.bullet_hole_mesh);
                }

                if (event.key.key == SDLK_LALT){
                    SDL_SetWindowRelativeMouseMode(window, lock_cursor);
                    lock_cursor = !lock_cursor;
                    if (lock_cursor){
                        SDL_HideCursor();
                    }
                    else{
                        SDL_ShowCursor();
                    }
                }
                if (event.key.key == SDLK_ESCAPE){
                    running = false;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                if (event.button.button == 1){
                    gun.is_shooting = true;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP){
                if (event.button.button == 1){
                    gun.is_shooting = false;
                }
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);

                onResize(w, h, camera);
            }

            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        player.solve_collisions(worldBVHs);
        
        CLEAR_SCREEN;
        
        player.update(dt, lock_cursor, sensitivity, camera);
        controller.root->origin = -camera.right * 0.25f + camera.position + -camera.up * 0.2f;
        controller.update(0.01, 10, 0.01);
        controller.set_arm_transform(arm, camera);
        gun.update(dt, camera, controller, player, worldBVHs);
        i_mesh.draw(program_instanced, camera.view, camera.proj, glm::vec4(1.0), tmp.texture);

        draw_level(level0, camera, program);
        draw_level(arm, camera, program);

        // draw gun (and thus bullet holes) at last 
        gun.draw(program, program_instanced, camera);

        if (lua_ctx) {
            // controller.goal = vec3(lua_ctx->goal_x, lua_ctx->goal_y, lua_ctx->goal_z);
        }

        // render_gizmos(camera); // automatically calls "end_frame_gizmos"
        end_frame_gizmos(); 

        imgui_frame(player);

        SDL_GL_SwapWindow(window);

        float target = 1000.0f / fps;
        float frame_ms = dt * 1000.0f;

        if (frame_ms < target) {
            SDL_Delay((Uint32)(target - frame_ms));
        }

        // printf("%.1f\n", frame_ms);
    }

    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(gl_context);
    SDL_Quit();
    imgui_shutdown();
    scripting_quit();
    audio_quit();

    return 0;
}
