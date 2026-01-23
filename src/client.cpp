#include <enet/enet.h>
#include <stdio.h>
#include <string.h>

int main()
{
    enet_initialize();
    atexit(enet_deinitialize);

    ENetHost* client = enet_host_create(
        NULL, // client
        1,
        2,
        0, 0
    );

    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;

    ENetPeer* serverPeer =
        enet_host_connect(client, &address, 2, 0);

    ENetEvent event;

    // espera conectar
    enet_host_service(client, &event, 3000);
    if (event.type == ENET_EVENT_TYPE_CONNECT)
        printf("conectado no server\n");
    else
        return 0;

    // manda msg
    ENetPacket* packet = enet_packet_create(
        "hello server",
        strlen("hello server") + 1,
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_peer_send(serverPeer, 0, packet);

    // loop pra receber resposta
    while (true)
    {
        while (enet_host_service(client, &event, 0) > 0)
        {
            if (event.type == ENET_EVENT_TYPE_RECEIVE)
            {
                printf("client recebeu: %s\n", event.packet->data);
                enet_packet_destroy(event.packet);
            }
        }
    }

    enet_host_destroy(client);
    return 0;
}
