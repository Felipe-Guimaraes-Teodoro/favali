#pragma once

#include <cstdint>
#include "glm.hpp"

enum PacketID : uint8_t
{
    PACKET_CONNECT,
    PACKET_INPUT,
    PACKET_SNAPSHOT, // world state
};

struct InputPacket
{
    uint8_t packetID = PACKET_INPUT;
    float moveX;
    float moveY;
    bool shoot;
};

struct PlayerState
{
    float x, y, z;
    float rotY;
};

struct SnapshotPacket {
    uint8_t packetID;
    float dt;
    glm::vec3 position;
    // put stuff in here to synchronize each client
};

