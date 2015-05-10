#pragma once

#include <unordered_set>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <components/Component.h>

class Entity;

class TriggerComponent : public Component {
public:
    TriggerComponent(btGhostObject *ghostObject);

    btGhostObject *getGhostObject() const;

    void addTriggerEntity(Entity *entity);
    void removeTriggerEntity(Entity *entity);
    void clearTriggerEntities();

    const std::unordered_set<Entity *> &getTriggerEntities() const;

private:
    btGhostObject *ghostObject;
    std::unordered_set<Entity *> triggerEntities;
};
