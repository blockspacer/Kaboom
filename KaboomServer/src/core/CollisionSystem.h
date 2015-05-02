#pragma once

#include "EntitySystem.h"

class CollisionSystem : public EntitySystem {
public:
    CollisionSystem(Game *game);

    virtual void update(float timeStep);
};
