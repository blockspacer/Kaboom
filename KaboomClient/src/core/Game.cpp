#include "Game.h"

#include <iostream>
#include <osg/Group>

#include <Core.h>
#include <core/Entity.h>
#include <core/PositionComponent.h>

#include "../Global.h"
#include "SceneNodeComponent.h"
#include "../Scene.h"
#include "../input/InputManager.h"
#include "../network/ClientEventHandlerLookup.h"
#include "../network/GameClient.h"

Game::Game(ConfigSettings *config)
    : config(config),
      playerFactory(entityManager),
      bombFactory(entityManager),
      eventHandlerLookup(new ClientEventHandlerLookup(this)) {
    client = new GameClient(eventHandlerLookup);

    std::string mediaPath, screenPosXStr, screenPosYStr, renbufferWStr, renbufferHStr, screenWStr, screenHStr;
    config->getValue(ConfigSettings::str_mediaFilePath, mediaPath);
    config->getValue(ConfigSettings::str_screenPosX, screenPosXStr);
    config->getValue(ConfigSettings::str_screenPosY, screenPosYStr);
    config->getValue(ConfigSettings::str_renderBuffer_width, renbufferWStr);
    config->getValue(ConfigSettings::str_renderBuffer_height, renbufferHStr);
    config->getValue(ConfigSettings::str_screen_width, screenWStr);
    config->getValue(ConfigSettings::str_screen_height, screenHStr);

    int posX = atoi(screenPosXStr.c_str());
    int posY = atoi(screenPosYStr.c_str());
    int bufferW = atoi(renbufferWStr.c_str());
    int bufferH = atoi(renbufferHStr.c_str());
    int screenW = atoi(screenWStr.c_str());
    int screenH = atoi(screenHStr.c_str());

    Core::init(posX, posY, screenW, screenH, bufferW, bufferH, mediaPath);
    setupScene();

    inputManager = new InputManager(*client);
    inputManager->loadConfig();

    Core::addEventHandler(&inputManager->getKeyboardEventHandler());
    Core::addEventHandler(&inputManager->getMouseEventHandler());
}

Game::~Game() {
    delete inputManager;
    delete eventHandlerLookup;
    delete client;
}

void Game::run() {
    static bool connected = false;

    Core::finalize();
    while (true) {
        if (Core::isInGameMode() && !connected) {
            std::string serverAddress;
            int serverPort;

            config->getValue(ConfigSettings::str_server_address, serverAddress);
            config->getValue(ConfigSettings::str_server_port, serverPort);

            client->connectToServer(serverAddress, serverPort);
            connected = true;
        }

        if (connected) {
            client->receive();
        }

        if (!Core::isInGameMode() && connected) {
            client->disconnectFromServer();
            connected = false;
        }

        Core::AdvanceFrame();
    }
}

bool Game::addSceneNodeEntity(Entity *entity) {
    SceneNodeComponent *sceneNodeCom = entity->getComponent<SceneNodeComponent>();
    if (sceneNodeCom == nullptr) {
       return false;
    }

	PositionComponent *posCom = entity->getComponent<PositionComponent>();
	if (posCom == nullptr) {
		return false;
	}
	
	Core::getWorldRef().getGeometryManager()->addGeometry(std::to_string(static_cast<int>(entity->getId())),
														  sceneNodeCom->getNode(), 
														  osg::Vec3(posCom->getX(), posCom->getY(), posCom->getZ()));

    //rootNode->addChild(sceneNodeCom->getNode());






    return true;
}

const EntityManager &Game::getEntityManager() const {
    return entityManager;
}

const PlayerFactory &Game::getPlayerFactory() const {
    return playerFactory;
}

const BombFactory &Game::getBombFactory() const {
    return bombFactory;
}

ClientEventHandlerLookup *Game::getEventHandlerLookup() const {
    return eventHandlerLookup;
}
