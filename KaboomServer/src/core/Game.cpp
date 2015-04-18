#include "Game.h"

#include <core/PositionComponent.h>
#include "PhysicsComponent.h"

Game::Game(ConfigSettings *config)
    : config(config),
    playerFactory(&entityManager) {
    server = new GameServer(config, this);

    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;

    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

    world->setGravity(btVector3(0, 0, -1)); // TODO: Extract gravity constant
}

Game::~Game() {
    delete server;

    delete broadphase;
    delete collisionConfiguration;
    delete dispatcher;
    delete solver;
}

void Game::loadMap() {
    // TODO: Handle memory leak
    btCollisionShape *groundShape = new btStaticPlaneShape(btVector3(0, 0, 1), 1);
    btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, -1)));

    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody *groundRigidBody = new btRigidBody(groundRigidBodyCI);

    world->addRigidBody(groundRigidBody);
}

void Game::addPhysicsEntity(Entity *entity) {
    PhysicsComponent *physicsCom = entity->getComponent<PhysicsComponent>();

    if (physicsCom != nullptr) {
        world->addRigidBody(physicsCom->getRigidBody());
    }
}

void Game::update(float timeStep) {

	//HERE is where the client first connect to server,
    //we want to have client load the gameworld first,
    //then create the player, and send the spawn player event to client
	if (server->acceptNewClient(entityManager.getNextId())) {

		//now we create a new player
        Entity *player = playerFactory.createPlayer(0, 0, 5);
        players.push_back(player);
        addPhysicsEntity(player);

		//notify client, a player spawn occurs
		server->sendPlayerSpawnEvent(player, players);

        //first have the client first preload the information about the world
        server->sendGameStatePackets(this);
    }

    server->receive();

    for (Entity *player : players) {
        player->getComponent<PhysicsComponent>()->getRigidBody()->activate(true);
    }

    world->stepSimulation(timeStep);
    
    for (Entity *player : players) {
        PositionComponent *positionCom = player->getComponent<PositionComponent>();
        PhysicsComponent *physicsCom = player->getComponent<PhysicsComponent>();

        const btVector3 &position = physicsCom->getRigidBody()->getWorldTransform().getOrigin();
        positionCom->setPosition(position.getX(), position.getY(), position.getZ());
    }

    server->sendGameStatePackets(this);
}

const EntityManager &Game::getEntityManager() const {
    return entityManager;
}
