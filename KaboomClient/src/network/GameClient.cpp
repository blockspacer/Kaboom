#include "GameClient.h"

#include <network/EmptyEvent.h>
#include <network/PlayerInputEvent.h>
#include <network/PositionEvent.h>
#include <network/RotationEvent.h>
#include <network/SpawnEvent.h>
#include <network/AssignEvent.h>

#include "NetworkServices.h"
#include "ClientEventHandlerLookup.h"

GameClient::GameClient(const ClientEventHandlerLookup &eventHandlerLookup)
        : eventHandlerLookup(eventHandlerLookup),
		  currentPlayerEntityId(0),
		  assignedEntity(false){
}

GameClient::~GameClient() {
}

bool GameClient::connectToServer(const std::string &serverAddress, const int serverPort) {
    network.connectToServer(serverAddress, serverPort);
	return network.isConnected();
}

bool GameClient::disconnectFromServer() {
	assignedEntity = false;//next time we connect, we need to obtain a new entity
    network.disconnectFromServer();
	return !network.isConnected();
}

void GameClient::receive() {
    char networkData[MAX_PACKET_SIZE];

    int len = network.receivePackets(networkData);

    if (len == 0) {
		printf("<Client> Server is not responding switch back to editor");
		network.disconnectFromServer();
        return;
	}
	if (len < 0){
		return;
	}

    EmptyEvent emptyEvent;
	AssignEvent assignEvent;
    PositionEvent positionEvent;
    RotationEvent rotationEvent;
    SpawnEvent spawnEvent;
    PlayerInputEvent playerInputEvent;
	DeleteEvent deleteEvent;
	

    // printf("received len %d\n", len);

    unsigned int i = 0;
    while (i < (unsigned int)len) {
        emptyEvent.deserialize(&networkData[i]);

		/*printf("eventType is %d\n", emptyEvent.getOpcode());
		printf("byteSize is %d\n", emptyEvent.getByteSize());*/

        switch (emptyEvent.getOpcode()) {
		case EventOpcode::ASSIGN_ENTITY:
			assignEvent.deserialize(&networkData[i]);
			if (!assignedEntity) {
				printf("<Client> Got Assigned a new EntityId %d\n", assignEvent.getEntityId());
				currentPlayerEntityId = assignEvent.getEntityId(); //set entityId the client needs to keep track of
			}
			break;
        case EventOpcode::POSITION:
            positionEvent.deserialize(&networkData[i]);
            eventHandlerLookup.find(emptyEvent.getOpcode())->handle(positionEvent);
            break;
        case EventOpcode::ROTATION:
            // TODO: Handle rotation event
            //rotationEvent.deserialize(&networkData[i]);
            //eventHandlerLookup.find(emptyEvent.getOpcode())->handle(rotationEvent);
            break;
        case EventOpcode::PLAYER_INPUT:
            playerInputEvent.deserialize(&networkData[i]);
            eventHandlerLookup.find(emptyEvent.getOpcode())->handle(playerInputEvent);
            break;
        case EventOpcode::ENTITY_SPAWN:
            spawnEvent.deserialize(&networkData[i]);
            eventHandlerLookup.find(emptyEvent.getOpcode())->handle(spawnEvent);

            break;
		case EventOpcode::DELETE_ENTITY:
			deleteEvent.deserialize(&networkData[i]);
			//create handler
			eventHandlerLookup.find(emptyEvent.getOpcode())->handle(deleteEvent);
			break;
        default:
            printf("error in packet event types\n");
            return;
        }

        i += emptyEvent.getByteSize();
    }
}

unsigned int GameClient::getCurrentPlayerEntityId() const{
	return currentPlayerEntityId;
}

bool GameClient::getAssignedEntity() const {
	return assignedEntity;
}
void GameClient::sendMessage(const Event &evt) {
    const int &size = evt.getByteSize();
    char *data = new char[size];

    evt.serialize(data);

    network.sendMessage(data, size);

    delete[] data;
}

bool GameClient::getIsConnectedToServer() const{
	return network.isConnected();
}
