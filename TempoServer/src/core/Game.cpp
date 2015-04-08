#include "Game.h"

Game::Game() {
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;

    world->setGravity(btVector3(0, 0, -10)); // TODO: Extract gravity constant
}

Game::~Game() {
    delete broadphase;
    delete collisionConfiguration;
    delete dispatcher;
    delete solver;
}

void Game::loadMap() {
    // TODO: Handle memory leak
    btCollisionShape *groundShape = new btStaticPlaneShape(btVector3(0, 0, 1), 0);
    btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody *groundRigidBody = new btRigidBody(groundRigidBodyCI);

    world->addRigidBody(groundRigidBody);
}

void Game::addPlayer(Player *player) {
	btRigidBody::btRigidBodyConstructionInfo playerRigidBodyCI(player->mass, player->motionState, Player::collisionShape, btVector3(0, 0, 0));
	btRigidBody *playerRigidBody = new btRigidBody(playerRigidBodyCI);

	world->addRigidBody(playerRigidBody)
}

void Game::update(float timeStep) {
	world->stepSimulation(timeStep);

	for (PlayerList::const_iterator it = players.cbegin(); it != players.cend(); ++it) {
		(*it)->update(timeStep);
	}
}
