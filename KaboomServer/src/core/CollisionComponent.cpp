#include "CollisionComponent.h"

CollisionComponent::CollisionComponent() {
}

void CollisionComponent::addContactEntity(Entity *entity) {
    contactEntities.insert(entity);
}

void CollisionComponent::clearContactEntities() {
    contactEntities.clear();
}

const bool &CollisionComponent::isCollided() const {
    return collided;
}

void CollisionComponent::setCollided(bool collided) {
    this->collided = collided;
}

const std::unordered_set<Entity *> &CollisionComponent::getContactEntities() const {
    return contactEntities;
}
