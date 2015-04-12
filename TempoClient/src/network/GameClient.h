#pragma once

#include "ClientNetwork.h"
#include "NetworkData.h"
#include "../core/GameStateData.h"

class GameClient {
public:
    GameClient(ConfigSettings *);
    ~GameClient();

    ClientNetwork *network;

    char networkData[MAX_PACKET_SIZE];

	GameStateData *receive();

    void sendMoveEvent(bool movingForward, bool movingBackward, bool movingLeft, bool movingRight);
};
