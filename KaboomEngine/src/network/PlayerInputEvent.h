#pragma once

#include "Event.h"

class PlayerInputEvent : public Event {
public:
    PlayerInputEvent(bool movingForward,
        bool movingBackward,
        bool movingLeft,
        bool movingRight,
        bool jumping,
        bool firing,
        float yaw,
        float pitch);
    ~PlayerInputEvent();

    const bool &getMovingForward() const;
    const bool &getMovingBackward() const;
    const bool &getMovingLeft() const;
    const bool &getMovingRight() const;

    const bool &getJumping() const;

    const bool &getFiring() const;

    const float &getYaw() const;
    const float &getPitch() const;

private:
    bool movingForward;
    bool movingBackward;
    bool movingLeft;
    bool movingRight;

    bool jumping;

    bool firing;

    float yaw;
    float pitch;
};
