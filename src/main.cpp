#include <memory>
#include <stdio.h>
#include <vector>

#include "texture.h"
#include "setups.h"
#include "player.h"
#include "gun.h"

#include "model.h"
#define UBO_DEFINITION
#include "shaders.h"

int w = 800;
int h = 600;

int main() {
    SDL_Window *window = nullptr;
    SDL_GLContext gl_context;
    
    init(&window, &gl_context, w, h);

    std::vector<unsigned int> textures;
    textures.push_back(create_texture("assets/bullet_hole.png"));

    Level *level = create_level_from_gltf("assets/level2.glb");
    std::vector<BVHNode*> worldBVHs;
    for (int i = 0; i < level->shapes.size(); i++){
        std::vector<MeshTriangle> world_tris = get_level_tris(&level->shapes[i]);
        worldBVHs.push_back(buildBVH(world_tris));
    }

    Camera camera = create_camera({10, 10, 10});
    Model *arm = create_model_from_gltf("../../../assets/arm.glb");
    IkController controller = create_ik_controller({0.0, 0.0, 0.0});
    int i = 0;
    for (const Shape &shape : arm->shapes) {
        controller.push_node(create_ik_node(shape.transform.scale.y * 2.0));
        i++;
    }

    int main_fs = create_shader(&default_fs, GL_FRAGMENT_SHADER);
    Lights lights = level->lights;
    UniformBuffer ubo = create_ubo<Lights>(&lights, sizeof(Lights));
    bind_ubo("Light", 0, ubo, main_fs);
    int main_vs = create_shader(&default_vs, GL_VERTEX_SHADER);
    int main_program = create_program(main_vs, main_fs);

    Sun sun = create_sun();

    Player player = create_player();

    Gun gun = create_gun(glm::vec3(7.07, 0.11, 0.), 2., 0., 1, textures[0]);
    // this MUST be callsed "nosin" or else...
    gun.shape = std::make_unique<Shape>(create_shape_from_gltf("assets/nosin.glb", 0));
    gun.bullet_template = create_bullet_template();
    
    SDL_Event event;
    bool running = true;
    bool lock_cursor = false;

    while(running){
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSize(window, &w, &h);

                on_resize(w, h, camera);
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
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
                if (event.button.button == 3){
                    gun.aiming = true;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP){
                if (event.button.button == 1){
                    gun.is_shooting = false;
                }
                if (event.button.button == 3){
                    gun.aiming = false;
                }
            }

            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        float dx, dy;
        SDL_GetRelativeMouseState(&dx, &dy);
        camera.mouse_view(lock_cursor, dx, dy, 0.1);
        camera.update();

        update_ubo<void>(ubo, &lights);

        player.solve_collisions(worldBVHs);
        player.update(0.0016, lock_cursor, 0.1, camera);
        controller.root->origin = -camera.right * 0.25f + camera.position + -camera.up * 0.2f;
        controller.update(0.01, 10, 0.01);
        controller.set_arm_transform(arm, camera);
        gun.update(0.0016, camera, controller, player, worldBVHs);

        CLEAR_SCREEN;

        draw_level(level, camera, main_program, &sun);
        gun.shape->draw(main_program, camera);
        draw_model(arm, camera, main_program);

        render_gizmos(camera);

        SDL_GL_SwapWindow(window);
    }

    end_process(&window, &gl_context);
}
