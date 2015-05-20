#include "Game.h"

#include <components/PositionComponent.h>
#include <components/RotationComponent.h>
#include <core/Entity.h>
#include <core/Player.h>
#include <util/ConfigSettings.h>

#include "../components/CollisionComponent.h"
#include "../components/InputComponent.h"
#include "../components/PhysicsComponent.h"
#include "../components/TriggerComponent.h"
#include "../network/GameServer.h"
#include "../network/ServerEventHandlerLookup.h"
#include "../systems/CollisionSystem.h"
#include "../systems/DeathSystem.h"
#include "../systems/DestroySystem.h"
#include "../systems/ExplosionSystem.h"
#include "../systems/FiringSystem.h"
#include "../systems/InitializationSystem.h"
#include "../systems/InputSystem.h"
#include "../systems/PhysicsSystem.h"
#include "../systems/PickupSystem.h"
#include "../systems/SpawnSystem.h"
#include "../systems/TimerSystem.h"
#include "../systems/VoidSystem.h"
#include "../systems/JumpPadSystem.h"

Game::Game(ConfigSettings *configSettings)
        : characterFactory(entityManager),
          bombFactory(entityManager),
          pickupFactory(entityManager),
		  jumpPadFactory(entityManager),
          eventHandlerLookup(this),
          server(configSettings, eventHandlerLookup),
          world(configSettings) {
    std::string str_mediaPath = "";
    std::string str_world_xml = "";

    configSettings->getValue(ConfigSettings::str_mediaFilePath, str_mediaPath);
    configSettings->getValue(ConfigSettings::str_world_xml, str_world_xml);

    str_world_xml = str_mediaPath + str_world_xml;

    std::cout << str_world_xml << std::endl;
    world.load(str_world_xml);

	//Adding the map objects and spawn points
	MapConfigLoader mapConfigLoader(mapConfigMap);
	mapConfigLoader.load("data-server/map.xml");

	for (auto mapConfig : mapConfigMap){
		if (mapConfig.second.getString("object-type") == "Pickup") {
			//duration is Zero at first, so the request is immediate and it will spawn right away in Spawn System
			pickupSpawnRequest.insert(std::make_pair(mapConfig.first, Timer(0)));
		} else if (mapConfig.second.getString("object-type") == "Player") {
			playerSpawnPointList.push_back(mapConfig.first);
		} else if (mapConfig.second.getString("object-type") == "JumpPad"){
			EntityType entityType = static_cast<EntityType>(mapConfig.second.getUint("id"));

			Vec3 boxSize = Vec3(mapConfig.second.getVec3("box-size").x(), mapConfig.second.getVec3("box-size").y(), mapConfig.second.getVec3("box-size").z());
			Vec3 positionVec = Vec3(mapConfig.second.getVec3("position").x(), mapConfig.second.getVec3("position").y(), mapConfig.second.getVec3("position").z());
			Vec3 launchDirection = Vec3(mapConfig.second.getVec3("launch-direction").x(), mapConfig.second.getVec3("launch-direction").y(), mapConfig.second.getVec3("launch-direction").z());
			Entity* jumpPadEntity = jumpPadFactory.createJumpPad(entityType,
				boxSize,
				positionVec,
				mapConfig.second.getFloat("launch-speed"),
				launchDirection,
				mapConfig.second.getFloat("launch-direction"));
			addEntity(jumpPadEntity);
		}
	}

    systemManager.addSystem(new InitializationSystem(this));
    systemManager.addSystem(new SpawnSystem(this));
    systemManager.addSystem(new InputSystem(this));
    systemManager.addSystem(new FiringSystem(this));
    systemManager.addSystem(new PhysicsSystem(this, world));
    systemManager.addSystem(new VoidSystem());
    systemManager.addSystem(new CollisionSystem(this));
    systemManager.addSystem(new TimerSystem(this));
    systemManager.addSystem(new PickupSystem(this));
    systemManager.addSystem(new ExplosionSystem(this));
	systemManager.addSystem(new JumpPadSystem(this));
    systemManager.addSystem(new DeathSystem(this));
    systemManager.addSystem(new DestroySystem(this));
}

Game::~Game() {
}

void Game::addEntity(Entity *entity) {
    PhysicsComponent *physicsComp = entity->getComponent<PhysicsComponent>();

    if (physicsComp != nullptr) {
        world.addRigidBody(physicsComp->getRigidBody());
    }

    TriggerComponent *triggerComp = entity->getComponent<TriggerComponent>();

    if (triggerComp != nullptr) {
        world.addTrigger(triggerComp->getGhostObject());
    }

    server.sendSpawnEvent(entity);
}

void Game::removeEntity(Entity *entity) {
    server.sendDestroyEvent(entity);

    PhysicsComponent *physicsComp = entity->getComponent<PhysicsComponent>();

    if (physicsComp != nullptr) {
        world.removeRigidBody(physicsComp->getRigidBody());
    }

    TriggerComponent *triggerComp = entity->getComponent<TriggerComponent>();

    if (triggerComp != nullptr) {
        world.removeTrigger(triggerComp->getGhostObject());
    }

    entityManager.destroyEntity(entity->getId());
}

void Game::update(float timeStep, int maxSubSteps) {

    //HERE is where the client first connect to server,
    //we want to have client load the gameworld first,
    //then create the player, and send the spawn player event to client
    if (server.acceptNewClient(entityManager.generateId())) {
        unsigned int playerId = playerIdPool.allocate();
        Entity *entity = characterFactory.createCharacter(DEFAULT_CHARACTER, Vec3(0.0f, -5.0f, 5.0f));

        entityIdToPlayer[entity->getId()] = new Player(playerId, entity);

        //first notify the new client what entityId it should keep track of
        server.sendAssignEvent(entity->getId());

        //send the new spawn player entity to all the clients
        addEntity(entity);

        //second send the new client about all the exisiting entities
        server.sendInitializeEvent(entity, entityManager.getEntityList());

        //lastly send the game state for each entity
        server.sendGameStatePackets(getEntityManager().getEntityList());
    }

    server.receive(this);

    systemManager.processSystems(this);

    server.sendGameStatePackets(getEntityManager().getEntityList());

    world.renderDebugFrame();
}
