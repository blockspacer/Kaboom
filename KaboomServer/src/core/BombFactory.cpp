#include "BombFactory.h"

#include <stdexcept>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <osgDB/XmlParser>

#include <components/PositionComponent.h>
#include <components/RotationComponent.h>
#include <core/EntityManager.h>
#include <util/XMLLoader.h>

#include "BombDataLookup.h"
#include "KaboomV2CollisionHandler.h"
#include "../components/CollisionComponent.h"
#include "../components/ExplosionComponent.h"
#include "../components/PhysicsComponent.h"
#include "../components/TriggerComponent.h"

BombFactory::BombFactory(EntityManager &entityManager)
        : entityManager(entityManager) {
}

Entity *BombFactory::createBomb(
        const EntityType &type,
        float x,
        float y,
        float z,
        float vx,
        float vy,
        float vz) const {
    const BombData &data = BombDataLookup::instance[type];

    Entity *entity = entityManager.createEntity(type);

    btTransform worldTrans;
    worldTrans.setIdentity();
    worldTrans.setOrigin(btVector3(x, y, z));

    btMotionState *motionState = new btDefaultMotionState(worldTrans);
    btCollisionShape *collisionShape = new btSphereShape(data.size);

    btRigidBody *rigidBody = new btRigidBody(data.mass, motionState, collisionShape, btVector3(0, 0, 0));
    rigidBody->setLinearVelocity(btVector3(vx, vy, vz));
    rigidBody->setUserPointer(entity);

    btGhostObject *ghostObject = new btGhostObject();
    ghostObject->setCollisionShape(new btSphereShape(data.explosionRadius));
    ghostObject->setWorldTransform(worldTrans);
    ghostObject->setUserPointer(entity);

    entity->attachComponent(new PositionComponent(x, y, z));
    entity->attachComponent(new RotationComponent());
    entity->attachComponent(new PhysicsComponent(rigidBody));
    entity->attachComponent(new TriggerComponent(ghostObject));
    entity->attachComponent(new CollisionComponent(data.collisionHandler));

    return entity;
}
