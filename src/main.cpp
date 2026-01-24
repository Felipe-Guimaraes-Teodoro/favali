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

int main() {
    SDL_Window *window = nullptr;
    SDL_GLContext gl_context;
    
    int w = 800;
    int h = 600;
    init(&window, &gl_context, w, h);

    std::vector<unsigned int> textures;
    textures.push_back(create_texture("assets/bullet_hole.png"));

    Level *level = create_level_from_gltf("assets/level2.glb");
    Model *target = create_model_from_gltf("assets/target.glb");

    for (auto& shape : target->shapes) {
        shape.transform.position.x += 10.0;
        level->shapes.push_back(shape_to_static(std::move(shape)));
    }

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

    int main_vs = create_shader(&default_vs, GL_VERTEX_SHADER);
    int main_fs = create_shader(&default_fs, GL_FRAGMENT_SHADER);
    int main_program = create_program(main_vs, main_fs);
    
    int instance_vs = create_shader(&default_vs_instanced, GL_VERTEX_SHADER);
    int instance_fs = create_shader(&default_fs_instanced, GL_FRAGMENT_SHADER);
    int instance_program = create_program(instance_vs, instance_fs);
    
    int ls_vs = create_shader(&laser_vs, GL_VERTEX_SHADER);
    int ls_fs = create_shader(&laser_fs, GL_FRAGMENT_SHADER);
    int laser_program = create_program(ls_vs, ls_fs);

    Lights lights = level->lights;
    UniformBuffer ubo = create_ubo<Lights>(&lights, sizeof(Lights));
    bind_ubo("Light", 0, ubo, main_fs);
    
    Sun sun = create_sun();

    Player player = create_player();

    // Gun gun = create_gun(glm::vec3(0.41949, 0.00111, -0.0019), 2., 0., 1, textures[0]); // for nosin
    Gun gun = create_gun(glm::vec3(0.1136, -0.0529, 0.0), 2., 0., 1, textures[0]); // for pistol
    // this MUST be callsed "nosin" or else...
    // gun.shape = std::make_unique<Shape>(create_shape_from_gltf("assets/nosin.glb", 0));
    gun.shape = std::make_unique<Shape>(create_shape_from_gltf("assets/pistol.glb", 0));

    gun.bullet_template = create_bullet_template();
    gun.cooldown = 0.1;
    
    SDL_Event event;

    Uint64 last = SDL_GetTicks();

    float fps = 60.0f;
    float dt = 0.0f;
    float time = 0.0f;
    float sensitivity = 0.1;
    static float recoil_timer = 0.0f;

    bool running = true;
    bool lock_cursor = false;

    while(running){
        Uint64 now = SDL_GetTicks();
        dt = (now - last)/1000.0f;
        
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
        player.update(dt, lock_cursor, sensitivity, camera);
        controller.root->origin = -camera.right * 0.25f + camera.position + -camera.up * 0.2f;
        controller.update(0.01, 10, 0.01);
        controller.set_arm_transform(arm, camera);
        gun.update(dt, camera, controller, player, worldBVHs);
        CLEAR_SCREEN;
        
        draw_level(level, camera, main_program, &sun);
        draw_model(arm, camera, main_program);
        
        gun.draw(main_program, instance_program, laser_program, camera);

        // render_gizmos(camera);

        SDL_GL_SwapWindow(window);

        float target = 1000.0f / fps;
        float frame_ms = dt * 1000.0f;

        last = now;

        if (frame_ms < target) {
            SDL_Delay((Uint32)(target - frame_ms));
        }

        // printf("%.1f\n", frame_ms);
        time += dt;

        // printf("%f\n", time);
    }

    end_process(&window, &gl_context);
}
