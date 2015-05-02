#pragma once

#include <typeinfo>
#include <unordered_map>

#include "EntityType.h"

class Component;

class Entity {
public:
    Entity(const unsigned int &id, const EntityType &type = EntityType::NONE);
    ~Entity();

    void attachComponent(Component *component);

    template <typename T>
    bool hasComponent() const {
        return components.count(&typeid(T)) > 0;
    }

    template <typename T>
    T *getComponent() const {
        if (hasComponent<T>()) {
            return static_cast<T *>(components.at(&typeid(T)));
        } else {
            return nullptr;
        }
    }

    const unsigned int &getId() const;

    const EntityType &getType() const;
    void setType(const EntityType &type);

private:
    unsigned int id;
    EntityType type;

    std::unordered_map<const std::type_info *, Component *> components;
};
