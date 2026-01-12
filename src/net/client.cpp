#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include "net_common.h"

#include <stdio.h>
#include <vector>
#include <memory>
#include <math.h>
#include "SDL3/SDL.h"
// #include "scripting.h"

#include "glad/glad.h"
#include "ui.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

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
#include "sun.h"
#include "PerlinNoise.hpp"
#include "npc.h"

static const char* SERVER_IP = "127.0.0.1";
static const int SERVER_PORT = 7777;
static const int CHANNELS = 2; // channels 0 and 1, use 0 for reliable and 1 for unreliable

ENetHost* init_client(){
	if (enet_initialize() != 0) {
		std::cerr << "erro ao iniciar enet\n";
		return nullptr;
	}

	ENetHost* client = enet_host_create(
		nullptr,
		1,
		CHANNELS,
		0,
		0
	);

	if (!client) {
		std::cerr << "erro ao criar client\n";
		enet_deinitialize();
		return nullptr;
	}

	return client;
}

InputPacket getPlayerInput() {
    InputPacket input{};
    input.packetID = PACKET_INPUT;

    const bool* keys = SDL_GetKeyboardState(NULL);
    input.moveX = keys[SDL_SCANCODE_A] ? -1.0f : keys[SDL_SCANCODE_D] ? 1.0f : 0.0f;
    input.moveY = keys[SDL_SCANCODE_W] ? 1.0f : keys[SDL_SCANCODE_S] ? -1.0f : 0.0f;
    input.shoot = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(1);
    
    return input;
}

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

    // imgui
    imgui_init(window, gl_context);
    
    // gl, our renderer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
        
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glViewport(0, 0, w, h);
    glClearColor(0.1, 0.2, 0.3, 1.0);

    init_gizmos();
}

void onResize(int w, int h, Camera& camera) {
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

int main() {
		ENetHost* client = init_client();

	ENetAddress address;
	enet_address_set_host(&address, SERVER_IP);
	address.port = SERVER_PORT;

	ENetPeer* peer = enet_host_connect(
		client,
		&address,
		CHANNELS,
		0
	);

	if (!peer)
	{
		std::cerr << "erro ao conectar no server\n";
		return 1;
	}

	std::cout << "conectando no server...\n";

	ENetEvent netEvent;

	if (enet_host_service(client, &netEvent, 5000) > 0 &&
		netEvent.type == ENET_EVENT_TYPE_CONNECT)
	{
		std::cout << "conectado com sucesso\n";

		const char* msg = "hello server";
		ENetPacket* packet = enet_packet_create(
			msg,
			strlen(msg) + 1,
			ENET_PACKET_FLAG_RELIABLE
		);

		enet_peer_send(peer, 0, packet);
		enet_host_flush(client);
	}
	else
	{
		std::cerr << "timeout ao conectar\n";
		enet_peer_reset(peer);
	}

	init();

    SDL_Event event;

    unsigned int vs = create_shader(&default_vs, GL_VERTEX_SHADER);
    unsigned int fs = create_shader(&default_fs, GL_FRAGMENT_SHADER);    
    unsigned int vs_depth = create_shader(&depth_vs, GL_VERTEX_SHADER);
    unsigned int fs_depth = create_shader(&depth_fs, GL_FRAGMENT_SHADER);
    unsigned int vs_instanced = create_shader(&default_vs_instanced, GL_VERTEX_SHADER);
    unsigned int fs_instanced = create_shader(&default_fs_instanced, GL_FRAGMENT_SHADER);
    unsigned int vs_cube_map = create_shader(&cube_map_vs, GL_VERTEX_SHADER);
    unsigned int fs_cube_map = create_shader(&cube_map_fs, GL_FRAGMENT_SHADER);
    unsigned int vs_sun = create_shader(&sun_vs, GL_VERTEX_SHADER);
    unsigned int fs_sun = create_shader(&sun_fs, GL_FRAGMENT_SHADER);

    unsigned int program = create_program(vs, fs);
    unsigned int depthProgram = create_program(vs_depth, fs_depth);
    unsigned int program_instanced = create_program(vs_instanced, fs_instanced);
    unsigned int program_cube_mesh = create_program(vs_cube_map, fs_cube_map);
    unsigned int program_sun = create_program(vs_sun, fs_sun);

    std::vector<unsigned int> textures;
    textures.push_back(make_texture("../../../../assets/bullet_hole.png"));
    textures.push_back(make_perlin_texture2D());

    CubeMapMesh sky = create_cube_map();
    sky.day_texture = make_cube_map_texture({
        "../../../../assets/skybox/day/day_right.jpg",
        "../../../../assets/skybox/day/day_left.jpg",
        "../../../../assets/skybox/day/day_top.jpg",
        "../../../../assets/skybox/day/day_bottom.jpg",
        "../../../../assets/skybox/day/day_front.jpg",
        "../../../../assets/skybox/day/day_back.jpg"
    });

    sky.night_texture = make_cube_map_texture({
        "../../../../assets/skybox/night/night_right.jpg",
        "../../../../assets/skybox/night/night_left.jpg",
        "../../../../assets/skybox/night/night_top.jpg",
        "../../../../assets/skybox/night/night_bottom.jpg",
        "../../../../assets/skybox/night/night_front.jpg",
        "../../../../assets/skybox/night/night_back.jpg"
    });

    Player localPlayer = create_player();

    audio_ctx->ud = &localPlayer;
    localPlayer.collider.radius = 1.0;
    
    glm::vec3 local_shoot_pos(3.2, -5.78, 0.);
    glm::vec3 muzzle_back_sample = glm::vec3(0., -5.78, 0.);
    Gun gun = make_gun(muzzle_back_sample, local_shoot_pos, 0.3f, 0.0f, 1, textures[0]);
    gun.shape = std::make_unique<Shape>(create_shape_from_gltf("../../../../assets/gun.gltf", 0));
    gun.shape->transform.scale = vec3(0.01);
    gun.bullet_template = create_bullet_template();

    Level *level0 = create_level_from_gltf("../../../../assets/level2.glb");
    
    Camera camera = create_camera({0, 0, 0}, 80.0, 1600.0 / 900.0);

    float sensitivity = 0.1;
    static float recoil_timer = 0.0f;

    Model *arm = create_model_from_gltf("../../../../assets/arm.glb");
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
	
	bool lock_cursor = false;
	float time = 0.0f;
	float dt = 0.0f;
	bool running = true;
	while (running)
	{
		InputPacket input = getPlayerInput();

		ENetPacket* packet = enet_packet_create(
			&input,
			sizeof(InputPacket),
			ENET_PACKET_FLAG_UNSEQUENCED  // input does not need to be reliable
		);

		enet_peer_send(peer, 1, packet);
		enet_host_flush(client);

		// process received events
		while (enet_host_service(client, &netEvent, 1) > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_RECEIVE: {
					SnapshotPacket* snap = (SnapshotPacket*)netEvent.packet->data;
					// apply snapshot onto local player
					localPlayer.position = snap->position;
					dt = snap->dt;
					
					enet_packet_destroy(netEvent.packet);
					break;
				}

				case ENET_EVENT_TYPE_DISCONNECT: {
					std::cout << "desconectado do server\n";
					running = false;
				}
			}
		}

		while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            if (event.type == SDL_EVENT_KEY_DOWN){
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
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSize(window, &w, &h);

                onResize(w, h, camera);
            }

            ImGui_ImplSDL3_ProcessEvent(&event);
        }

		sky.sun.updateSun(time);

        sky.sun.renderShadow(depthProgram, {level0}, w, h);

        CLEAR_SCREEN;
		// interpolate player position locally to the one received frmo server to make it crisp and juicy and ohhh yeah hmmm yes delicious yummy oh my god so juicy yes
        // localPlayer.update(dt, lock_cursor, sensitivity, camera);
        update_audio(camera.position, camera.front);

        controller.root->origin = -camera.right * 0.25f + camera.position + -camera.up * 0.2f;
        controller.update(0.01, 10, 0.01);
        controller.set_arm_transform(arm, camera);
		// no guns for now i dont like violence
        // gun.update(dt, camera, controller, localPlayer, worldBVHs);

        draw_level(level0, camera, program, &sky.sun);
        draw_model(arm, camera, program);

        sky.draw(program_cube_mesh, program_sun, camera, time); // last to draw for optimisation purposes

        // draw gun (and thus bullet holes) at last 
        gun.draw(program, program_instanced, camera);

        sky.draw(program_cube_mesh, program_sun, camera, time); // last to draw for optimisation purposes

        // if (lua_ctx) {
        //     // controller.goal = vec3(lua_ctx->goal_x, lua_ctx->goal_y, lua_ctx->goal_z);
        // }

        render_gizmos(camera); // automatically calls "end_frame_gizmos"
        // end_frame_gizmos(); 

        imgui_frame(localPlayer);

        SDL_GL_SwapWindow(window);
		time += dt;
	}

	enet_host_destroy(client);
	enet_deinitialize();
	return 0;
}
