#include "GameClient.h"

#include <network/AmmoAmountEvent.h>
#include <network/AssignEvent.h>
#include <network/BindEvent.h>
#include <network/ChatEvent.h>
#include <network/ConnectEvent.h>
#include <network/DestroyEvent.h>
#include <network/DisconnectEvent.h>
#include <network/EmptyEvent.h>
#include <network/EquipEvent.h>
#include <network/ExplosionEvent.h>
#include <network/MatchStateEvent.h>
#include <network/PlayerDeathEvent.h>
#include <network/PlayerInputEvent.h>
#include <network/PlayerRenameEvent.h>
#include <network/PlayerRespawnEvent.h>
#include <network/PlayerStatusEvent.h>
#include <network/PlayerRespawnRequestEvent.h>
#include <network/PositionEvent.h>
#include <network/ReloadRequestEvent.h>
#include <network/RotationEvent.h>
#include <network/ScoreEvent.h>
#include <network/SpawnEvent.h>'

#include "NetworkServices.h"
#include "ClientEventHandlerLookup.h"

GameClient::GameClient(const ClientEventHandlerLookup &eventHandlerLookup)
        : eventHandlerLookup(eventHandlerLookup) {
}

bool GameClient::connectToServer(const std::string &serverAddress, const int serverPort) {
    network.connectToServer(serverAddress, serverPort);
    return network.isConnected();
}

bool GameClient::disconnectFromServer() {
    network.disconnectFromServer();
    return !network.isConnected();
}

void GameClient::receive() {
    static char networkBuffer[MAX_PACKET_SIZE];
    static int bufferOffset = 0;

    int result = network.receivePackets(networkBuffer + bufferOffset, MAX_PACKET_SIZE - bufferOffset);
    int len = bufferOffset + result;

    bufferOffset = 0;

    if (result == 0) {
        printf("<Client> Server is not responding switch back to editor");
        network.disconnectFromServer();
        return;
    }

    if (result < 0) {
        return;
    }

    int i = 0;
    while (i < len) {
        EmptyEvent emptyEvent;
        emptyEvent.deserialize(&networkBuffer[i]);

        if (i + sizeof(EmptyEvent) > len) {
            bufferOffset = len - i;
            memcpy(networkBuffer, &networkBuffer[i], bufferOffset);
            break;
        } else if (i + sizeof(EmptyEvent) > MAX_PACKET_SIZE || i + emptyEvent.getByteSize() > MAX_PACKET_SIZE) {
            bufferOffset = MAX_PACKET_SIZE - i;
            memcpy(networkBuffer, &networkBuffer[i], bufferOffset);
            break;
        }

       // printf("eventType is %d\n", emptyEvent.getOpcode());
        //printf("byteSize is %d\n", emptyEvent.getByteSize());

        switch (emptyEvent.getOpcode()) {
            case EVENT_CONNECT: {
                ConnectEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_DISCONNECT: {
                DisconnectEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_ASSIGN: {
                AssignEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_BIND: {
                BindEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_MATCH_STATE: {
                MatchStateEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_CHAT: {
                ChatEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_SCORE: {
                ScoreEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_PLAYER_RENAME: {
                PlayerRenameEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_PLAYER_RESPAWN: {
                PlayerRespawnEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_PLAYER_DEATH: {
                PlayerDeathEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_SPAWN: {
                SpawnEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_DESTROY: {
                DestroyEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_POSITION: {
                PositionEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_ROTATION: {
                RotationEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_EXPLOSION: {
                ExplosionEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_HEALTH: {
                HealthEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_AMMO_COUNT: {
                AmmoAmountEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            case EVENT_PLAYER_STATUS: {
                PlayerStatusEvent evt;
                evt.deserialize(&networkBuffer[i]);
                eventHandlerLookup.find(evt.getOpcode())->handle(evt);
                break;
            }
            default: {
                printf("error in packet event types: %d\n", emptyEvent.getOpcode());
                return;
            }
        }

        i += emptyEvent.getByteSize();
    }
}

void GameClient::sendMessage(const Event &evt) const {
    const unsigned int size = evt.getByteSize();
    char *data = new char[size];

    evt.serialize(data);

    network.sendMessage(data, size);

    delete[] data;
}

void GameClient::sendPlayerRenameEvent(const std::string &name) const {
    PlayerRenameEvent evt(0, name.c_str());
    sendMessage(evt);
}

void GameClient::sendEquipEvent(EntityType type) const {
    EquipEvent evt(0, type);
    sendMessage(evt);
}

void GameClient::sendReloadRequestEvent() const {
    ReloadRequestEvent evt;
    sendMessage(evt);
}

bool GameClient::getIsConnectedToServer() const {
    return network.isConnected();
}

void GameClient::sendPlayerRespawnRequestEvent() const {
	PlayerRespawnRequestEvent evt;
	sendMessage(evt);
}
