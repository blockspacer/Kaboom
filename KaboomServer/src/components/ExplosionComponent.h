#pragma once

#include <core/Component.h>

#include "../core/ExplosionHandler.h"

class ExplosionComponent : public Component {
public:
    ExplosionComponent(ExplosionHandler *handler);

    ExplosionHandler *getHandler() const;

private:
    ExplosionHandler *handler;
};
