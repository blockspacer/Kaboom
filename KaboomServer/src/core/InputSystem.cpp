#include "InputSystem.h"

#include <btBulletDynamicsCommon.h>

#include <core/EntityManager.h>
#include <core/PositionComponent.h>
#include <core/RotationComponent.h>
#include <core/PlayerStatusComponent.h>

#include "Game.h"
#include "InputComponent.h"
#include "PhysicsComponent.h"
#include "JetpackComponent.h"
#include "../math/Conversion.h"

#define VELOCITYCAP 2
#define VELOCTIYACCELERATION .1

InputSystem::InputSystem(Game *game)
        : EntitySystem(game) {
}

void InputSystem::update(float timeStep) {
    for (Entity *entity : game->getEntityManager().getEntityList()) {
        InputComponent *inputCom = entity->getComponent<InputComponent>();
        PositionComponent *posCom = entity->getComponent<PositionComponent>();
        RotationComponent *rotCom = entity->getComponent<RotationComponent>();
        PhysicsComponent *physCom = entity->getComponent<PhysicsComponent>();
        PlayerStatusComponent *playerStatusCom = entity->getComponent<PlayerStatusComponent>();

        if (inputCom == nullptr || physCom == nullptr || playerStatusCom == nullptr) {
            continue;
        }

        btRigidBody *rigidBody = physCom->getRigidBody();
        btVector3 velocity = rigidBody->getLinearVelocity();

        // TODO: currently, forward & backward event will override 
        // the front and right velocity generated by the physics.
        // Only velocity on the z axis is taken from the physics calc

        btVector3 viewDir = getViewDirection(
            posCom->getX(),
            posCom->getY(),
            posCom->getZ(),
            rotCom->getYaw(),
            rotCom->getPitch());

        // construct local coordinate system
        btVector3 front = viewDir;
        btVector3 right = front.cross(btVector3(0, 0, 1));
        right.normalize();
        // do not calculate head vector, since gravity is always neg z

        btVector3 localVelocity(0.0, 0.0, 0.0);

        // Define y to be front speed, x to be right speed
        if (inputCom->isMovingForward()) {
            localVelocity.setY(1);
        } else if (inputCom->isMovingBackward()) {
            localVelocity.setY(-1);
        } else {
            localVelocity.setY(0);
        }

        if (inputCom->isMovingLeft()) {
            localVelocity.setX(-1);
        } else if (inputCom->isMovingRight()) {
            localVelocity.setX(1);
        } else {
            localVelocity.setX(0);
        }

        JetpackComponent *jetComp = entity->getComponent<JetpackComponent>();

        if (jetComp != nullptr) {
            if (inputCom->isJumping()) {

                if (!jetComp->activateJetpack()) {
                    jetComp->refillJetpack();
                } else {
                    if (velocity.getZ() > VELOCITYCAP) {
                        velocity.setZ(VELOCITYCAP);
                    } else {
                        velocity.setZ(velocity.getZ() + VELOCTIYACCELERATION);
                    }
                }
            } else {
                jetComp->refillJetpack();
            }
        }

        btVector3 worldVelocity(right * localVelocity.getX() + front * localVelocity.getY());
        worldVelocity.setZ(velocity.getZ());

        if (!playerStatusCom->checkIsKnockBacked()) { //don't move if we are knockbacked
            rigidBody->setLinearVelocity(worldVelocity);
        }
    }
}
