#include <enet/enet.h>
#include <iostream>
#include "net_common.h"
#include "player.h"
#include "level.h"
#include "gun.h"
#include <model.hpp>
#include <chrono>
#include "graphics/mesh.h"

static const int SERVER_PORT = 7777;
static const int CHANNELS = 2;

int main() {
    if (enet_initialize() != 0) {
        std::cerr << "Erro ao iniciar ENet\n";
        return 1;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;  // aceita qualquer IP
    address.port = SERVER_PORT;

    ENetHost* server = enet_host_create(
        &address,
        32,           // max clients
        CHANNELS,     // canais
        0, 0
    );

    if (!server) {
        std::cerr << "Erro ao criar server\n";
        enet_deinitialize();
        return 1;
    }

    Level *level0 = create_level_from_gltf("../../../assets/level2.glb");
    std::vector<BVHNode*> worldBVHs;
    for (int i = 0; i < level0->shapes.size(); i++){
        std::vector<MeshTriangle> world_tris = get_level_tris(&level0->shapes[i]);
        worldBVHs.push_back(buildBVH(world_tris));
    }

    std::vector<Player> players;  // vetor de players conectados
    float dt = 0.0; 
    auto last = std::chrono::steady_clock::now();   
    
    bool running = true;
    while (running) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        ENetEvent event;
        while (enet_host_service(server, &event, 1) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    std::cout << "Novo player conectado\n";
                    Player p = create_player(); // cria player com posição inicial
                    players.push_back(p);
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    uint8_t* data = event.packet->data;
                    uint8_t packetID = data[0];  //first byte = packet ID

                    if (packetID == PACKET_INPUT) {
                        InputPacket* input = (InputPacket*) data;

                        // TODO: applly input only on corresponding player
                        Player& p = players[0];
                        p.applyInput(*input, dt);

                        if (input->shoot) {
                            // pew pew
                        }
                    }
                    enet_packet_destroy(event.packet);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    std::cout << "Player desconectou\n";
                    // remover player da lista se quiser
                    break;
                }

                default:
                    break;
            }
        }

        // world update
        for (auto& p : players) {
            // p.solve_collisions();
        }

        // send snapshot to all players
        for (auto& p : players) {
            SnapshotPacket snap{};
            snap.packetID = PACKET_SNAPSHOT;
            snap.dt = dt;
            snap.position = p.position;

            ENetPacket* packet = enet_packet_create(&snap, sizeof(snap), ENET_PACKET_FLAG_UNSEQUENCED);
            enet_host_broadcast(server, 1, packet); // fast, unreliable channel cuz we rock like that
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();
    return 0;
}
