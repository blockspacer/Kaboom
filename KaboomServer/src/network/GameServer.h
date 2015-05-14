#pragma once

#include <components/PositionComponent.h>
#include <components/RotationComponent.h>
#include <components/BombContainerComponent.h>
#include <components/HealthComponent.h>
#include <components/PlayerStatusComponent.h>

#include <network/HealthEvent.h>
#include <network/NetworkData.h>

#include "../components/PhysicsComponent.h"

#include "ServerEventHandlerLookup.h"
#include "ServerNetwork.h"
#include "NetworkServices.h"

class Entity;
class Game;

class GameServer {
public:
    GameServer(ConfigSettings *, const ServerEventHandlerLookup &eventHandlerLookup);

    bool acceptNewClient(unsigned int);

    void receive(Game *game);

    void sendEvent(const Event &evt) const;
    void sendEvent(const Event &evt, const unsigned int &clientId) const;

    void sendAssignEvent(const unsigned int &entityId) const;
    void sendDisconnectEvent(Entity *entity) const;
    void sendSpawnEvent(Entity *entity) const;
    void sendDestroyEvent(Entity *entity) const;
    void sendPositionEvent(Entity *entity) const;
    void sendRotationEvent(Entity *entity) const;
    void sendExplosionEvent(Entity *bomb) const;
	void sendHealthEvent(Entity *entity) const;
    void sendAmmoEvent(Entity *entity) const;
	void sendPlayerStatusEvent(Entity *entity) const;

    void sendInitializeEvent(Entity* player, const std::vector<Entity *> &entities) const;
    void sendGameStatePackets(const std::vector<Entity *> &entities) const;

private:
    const ServerEventHandlerLookup &eventHandlerLookup;
	unsigned int nextClientId;
	unsigned int currClientId;
	std::unordered_map<unsigned int, unsigned int> clientIdToEntityId;
	std::unordered_map<unsigned int, unsigned int> entityIdToClientId;

    ServerNetwork* network;
};