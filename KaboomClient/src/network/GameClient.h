#pragma once

#include <network/Event.h>
#include <network/NetworkData.h>

#include "ClientNetwork.h"

class ClientEventHandlerLookup;

class GameClient {
public:
    GameClient(ClientEventHandlerLookup *eventHandlerLookup);
    ~GameClient();

	void receive();
    bool connectToServer(const std::string &serverAddress, const int serverPort);
	bool disconnectFromServer();

    void sendMessage(const Event &evt);

private:
    ClientEventHandlerLookup *eventHandlerLookup;

    ClientNetwork network;

    bool initialized;

	unsigned int currentPlayerEntityId;
};
