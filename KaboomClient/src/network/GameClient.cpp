#include "GameClient.h" 

#include "NetworkServices.h"

GameClient::GameClient(ConfigSettings *config, ClientEventHandlerLookup *eventHandlerLookup) :
    eventHandlerLookup(eventHandlerLookup) {
    network = new ClientNetwork(config);
}

GameClient::~GameClient() {
    delete network;
}

GameStateData *GameClient::receive() {
    //Packet packet;

    int len = network->receivePackets(networkData);

    if (len <= 0) {
        return nullptr;
    }

	Event emptyEvent = Event();
	PlayerSpawnEvent playerSpawnEvent = PlayerSpawnEvent();
	PlayerInputEvent playerInputEvent = PlayerInputEvent();
	PositionEvent positionEvent = PositionEvent();
	RotationEvent rotationEvent = RotationEvent();

	
	//GameStateData *gameStateData = new GameStateData();
	//GameStateUpdateEvent gameStateUpdateEvent; 
	//AssignEvent as;

	//bool recievedGameStateUpdateEvent = false;
	printf("received len %d\n", len);

	unsigned int i = 0;
	while (i < (unsigned int)len) {
		emptyEvent.deserialize(&(networkData[i]));

		printf("eventType is %d\n", emptyEvent.getOpcode());
		printf("byteSize is %d\n", emptyEvent.getByteSize());


		switch (emptyEvent.getOpcode()) {
		case EventOpcode::PLAYER_SPAWN:

			playerSpawnEvent.deserialize(&(networkData[i]));
			/*
			printf("getPlayerId is %d\n", playerSpawnEvent.getPlayerId());
			printf("getX is %f\n", playerSpawnEvent.getX());
			printf("getY is %f\n", playerSpawnEvent.getY());
			printf("getZ is %f\n", playerSpawnEvent.getZ());*/
			eventHandlerLookup->find(emptyEvent.getOpcode())->handle(playerSpawnEvent);
			
			break;

		case EventOpcode::PLAYER_INPUT:
			playerInputEvent.deserialize(&(networkData[i]));
			eventHandlerLookup->find(emptyEvent.getOpcode())->handle(playerInputEvent);
			break;
		case EventOpcode::POSITION:
			positionEvent.deserialize(&(networkData[i]));
			eventHandlerLookup->find(emptyEvent.getOpcode())->handle(positionEvent);

			break;
		case EventOpcode::ROTATION:
			rotationEvent.deserialize(&(networkData[i]));
			eventHandlerLookup->find(emptyEvent.getOpcode())->handle(rotationEvent);
			break;
		default:
			printf("error in packet event types\n");
			return nullptr;
		}


		/*
		printf("received len %d\n", len);
		packet.deserialize(&(networkData[i]));

		switch (packet.packet_type) {
		case ASSIGN_EVENT:
			as.deserialize(&(networkData[i]));
			printf("\nplayer is %d\n", as.client_id);
			break;

		case GAME_STATE_UPDATE_EVENT:
			recievedGameStateUpdateEvent = true;
			
			//gameStateUpdateEvent->deserialize(networkData);
			
			gameStateUpdateEvent.deserialize(&(networkData[i]));

			gameStateData->x1 = gameStateUpdateEvent.playerList[0].x;
			gameStateData->y1 = gameStateUpdateEvent.playerList[0].y;
			gameStateData->z1 = gameStateUpdateEvent.playerList[0].z;
			if (gameStateUpdateEvent.numOfPlayers != 2){
				gameStateData->x2 = 0;
				gameStateData->y2 = 0;
				gameStateData->z2 = 0;
			}
			else{
				gameStateData->x2 = gameStateUpdateEvent.playerList[1].x;
				gameStateData->y2 = gameStateUpdateEvent.playerList[1].y;
				gameStateData->z2 = gameStateUpdateEvent.playerList[1].z;
			}

			printf("[0] client id : %d", gameStateUpdateEvent.playerList[0].client_id);
			printf("[1] client id : %d", gameStateUpdateEvent.playerList[1].client_id);
			printf("[2] client id : %d", gameStateUpdateEvent.playerList[2].client_id);
			printf("[3] client id : %d", gameStateUpdateEvent.playerList[3].client_id);
			printf("\nx1 %x\n", gameStateData->x1);
			printf("y1 %x\n", gameStateData->y1);
			printf("z1 %x\n", gameStateData->z1);
			printf("x2 %x\n", gameStateData->x2);
			printf("y2 %x\n", gameStateData->y2);
			printf("z2 %x\n", gameStateData->z2);

			break;
		default:
			printf("error in packet types\n");
			return nullptr;
		}
		*/
		i += emptyEvent.getByteSize();
		//printf("new i is %d\n", i);
	}
	/*
	if (recievedGameStateUpdateEvent) {
		return gameStateData;
	}
	else {
		delete gameStateData;
        return nullptr;
	}*/

	return nullptr;
}

void GameClient::sendMoveEvent(bool movingForward, bool movingBackward, bool movingLeft, bool movingRight) {
    const unsigned int size = sizeof(MoveEvent);
    char data[size];
	cout << "enter the sendMoveEvent" << endl;
    MoveEvent packet;
	packet.packet_type = MOVE_EVENT;
    packet.movingForward = movingForward;
    packet.movingBackward = movingBackward;
    packet.movingLeft = movingLeft;
    packet.movingRight = movingRight;

    packet.serialize(data);

    NetworkServices::sendMessage(network->clientSocket, data, size);
}
