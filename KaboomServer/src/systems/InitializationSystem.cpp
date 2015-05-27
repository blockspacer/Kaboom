#include "InitializationSystem.h"

#include <components/PlayerStatusComponent.h>
#include <components/PositionComponent.h>
#include <components/RotationComponent.h>
#include <core/EntityManager.h>

#include "../components/CharacterRotationComponent.h"
#include "../components/CollisionComponent.h"
#include "../components/DestroyComponent.h"
#include "../components/PhysicsComponent.h"
#include "../components/TriggerComponent.h"
#include "../core/Game.h"

InitializationSystem::InitializationSystem(Game *game)
        : EntityProcessingSystem(game) {
}

bool InitializationSystem::checkEntity(Entity *entity) {
    return true;
}

void InitializationSystem::processEntity(Entity *entity) {
    // Clear collision results
    CollisionComponent *colComp = entity->getComponent<CollisionComponent>();

    if (colComp != nullptr) {
        colComp->setCollided(false);
        colComp->clearContactEntities();
    }

    // Clear trigger results
    TriggerComponent *triggerComp = entity->getComponent<TriggerComponent>();

    if (triggerComp != nullptr) {
        triggerComp->clearTriggerEntities();
    }

    // Activate rigid bodies
    PhysicsComponent *physComp = entity->getComponent<PhysicsComponent>();

    if (physComp != nullptr) {
        btRigidBody *rigidBody = entity->getComponent<PhysicsComponent>()->getRigidBody();
        rigidBody->activate(true);

        // Update rotation
        RotationComponent *rotComp = entity->getComponent<RotationComponent>();
        
        if (rotComp != nullptr) {
            Quat rot = rotComp->getRotation();
            rigidBody->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
        }
    }

    // Update trigger position
    if (triggerComp != nullptr) {
        btTransform worldTrans = btTransform::getIdentity();

        if (physComp != nullptr) {
            worldTrans = physComp->getRigidBody()->getWorldTransform();
        } else {
            PositionComponent *posComp = entity->getComponent<PositionComponent>();

            if (posComp != nullptr) {
                const Vec3 &pos = posComp->getPosition();
                worldTrans.setOrigin(btVector3(pos.x, pos.y, pos.z));
            }
        }

        triggerComp->getGhostObject()->setWorldTransform(worldTrans);
    }

    // Reset player status
    PlayerStatusComponent *playerStatusComp = entity->getComponent<PlayerStatusComponent>();

    if (playerStatusComp != nullptr) {
        playerStatusComp->setDamaged(false);
    }
}
