#include <enet/enet.h>
#include <stdio.h>

int main()
{
    enet_initialize();
    atexit(enet_deinitialize);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1234;

    ENetHost* server = enet_host_create(
        &address, // bind
        32,       // max clients
        2,        // canais
        0, 0
    );

    printf("server rodando na porta 1234\n");

    ENetEvent event;

    while (true)
    {
        while (enet_host_service(server, &event, 0) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("client conectou\n");
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    printf("server recebeu: %s\n", event.packet->data);
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("client desconectou\n");
                    break;
            }
        }
    }

    enet_host_destroy(server);
    return 0;
}
