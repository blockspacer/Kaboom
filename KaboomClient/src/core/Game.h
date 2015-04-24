#pragma once

#include <ConfigSettings.h>
#include <core/EntityManager.h>

#include "BombFactory.h"
#include "PlayerFactory.h"
#include "../network/ClientEventHandlerLookup.h"
#include "../network/GameClient.h"

class InputManager;

enum GameStateMachine {
	EDITOR_MODE,
	CONNECT_TO_SERVER,
	GAME_MODE,
	DISCONNECT_TO_SERVER
};

class Game {
public:
    Game(ConfigSettings *config);
    ~Game();

    void run();

    bool addSceneNodeEntity(Entity *entity);

    const EntityManager &getEntityManager() const;
    const PlayerFactory &getPlayerFactory() const;
	const BombFactory &getBombFactory() const;

private:
    ConfigSettings *config;
    InputManager *inputManager;

    EntityManager entityManager;
    PlayerFactory playerFactory;
	BombFactory bombFactory;

    ClientEventHandlerLookup eventHandlerLookup;
    GameClient client;

    // osgViewer::Viewer viewer;
    // osg::Group *rootNode;
    // void setupCamera();
};
