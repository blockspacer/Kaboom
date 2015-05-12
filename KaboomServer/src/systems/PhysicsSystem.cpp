#include "PhysicsSystem.h"

#include <components/PositionComponent.h>
#include <components/RotationComponent.h>

#include "../components/CollisionComponent.h"
#include "../components/StickComponent.h"
#include "../core/Game.h"

PhysicsSystem::PhysicsSystem(Game *game, World &world)
        : game(game),
          world(world) {
}

void PhysicsSystem::preprocessEntities(std::vector<Entity *> entities) {
    const clock_t FPS = 60;
    const float TIME_STEP = 1.0f / FPS;
    const int MAX_SUB_STEPS = 1;

    world.stepSimulation(TIME_STEP, MAX_SUB_STEPS); // TODO: Extract constant
}

bool PhysicsSystem::checkEntity(Entity *entity) {
    return entity->hasComponent<PhysicsComponent>();
}

void PhysicsSystem::processEntity(Entity *entity) {
    auto physComp = entity->getComponent<PhysicsComponent>();
    auto posComp = entity->getComponent<PositionComponent>();
    auto colComp = entity->getComponent<CollisionComponent>();

    btTransform &worldTrans = physComp->getRigidBody()->getWorldTransform();
    bool sticked = colComp != nullptr && colComp->isCollided() && entity->hasComponent<StickComponent>();

    if (posComp != nullptr) {
        if (sticked) {
            worldTrans.setOrigin(btVector3(posComp->getX(), posComp->getY(), posComp->getZ()));
        } else {
            const btVector3 &pos = worldTrans.getOrigin();
            posComp->setPosition(pos.getX(), pos.getY(), pos.getZ());
        }
    }

    RotationComponent *rotComp = entity->getComponent<RotationComponent>();

    if (rotComp != nullptr) {
        if (sticked) {
            // TODO: Set rotation
        } else {
            btQuaternion rot = worldTrans.getRotation();
            // TODO: Set rotComp
        }
    }
}
