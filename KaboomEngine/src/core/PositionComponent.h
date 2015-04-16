#pragma once

#include "Component.h"

class PositionComponent : public Component {
public:
    PositionComponent();
    PositionComponent(float x, float y, float z);
    ~PositionComponent();

    float getX() const;
    float getY() const;
    float getZ() const;

    void setX(float x);
    void setY(float y);
    void setZ(float z);

    void setPosition(float x, float y, float z);

private:
    float x;
    float y;
    float z;
};
