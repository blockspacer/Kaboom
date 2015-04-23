#include "InputComponent.h"

InputComponent::InputComponent()
    : InputComponent(false, false, false, false, false, false) {
}

InputComponent::InputComponent(bool movingForward,
        bool movingBackward,
        bool movingLeft,
        bool movingRight,
        bool jumping,
        bool firing)
        : movingForward(movingForward),
          movingBackward(movingBackward),
          movingLeft(movingLeft),
          movingRight(movingRight),
          jumping(jumping),
          firing(firing) {
}

InputComponent::~InputComponent() {
}

const bool &InputComponent::isMovingForward() const {
    return movingForward;
}

const bool &InputComponent::isMovingBackward() const {
    return movingBackward;
}

const bool &InputComponent::isMovingLeft() const {
    return movingLeft;
}

const bool &InputComponent::isMovingRight() const {
    return movingRight;
}

const bool &InputComponent::isJumping() const {
    return jumping;
}

const bool &InputComponent::isFiring() const {
    return firing;
}

void InputComponent::setMovingForward(const bool &movingForward) {
    this->movingForward = movingForward;
}

void InputComponent::setMovingBackward(const bool &movingBackward) {
    this->movingBackward = movingBackward;
}

void InputComponent::setMovingLeft(const bool &movingLeft) {
    this->movingLeft = movingLeft;
}

void InputComponent::setMovingRight(const bool &movingRight) {
    this->movingRight = movingRight;
}

void InputComponent::setJumping(const bool &jumping) {
    this->jumping = jumping;
}

void InputComponent::setFiring(const bool &firing) {
    this->firing = firing;
}
