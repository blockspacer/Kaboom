#pragma once

#include <network/NetworkData.h>
#include <network/DeleteEvent.h>
#include <core/BombContainerComponent.h>
#include "ServerEventHandlerLookup.h"
#include "ServerNetwork.h"
#include "NetworkServices.h"
#include "../core/PhysicsComponent.h"

class Entity;
class Game;

class GameServer {
public:
    GameServer(ConfigSettings *, const ServerEventHandlerLookup &eventHandlerLookup);
    ~GameServer();

    bool acceptNewClient(unsigned int);

    void receive(Game *game);

    void sendGameStatePackets(Game *game) const;

    void sendPositionEvent(Entity *entity) const;
	void sendEntitySpawnEvent(Entity* newEntity) const;
    void sendAllEntitiesSpawnEvent(Entity* newEntity, std::vector<Entity *> players) const;

	void sendAssignPlayerEntity(unsigned int);

private:
    const ServerEventHandlerLookup &eventHandlerLookup;

	// IDs for the clients connecting for table in ServerNetwork 
	static unsigned int client_id;

	// The ServerNetwork object 
	ServerNetwork* network;
};