#include "InputManager.h"

#include <network/PlayerInputEvent.h>

#include "../Global.h"

bool InputManager::movingForward;
bool InputManager::movingBackward;
bool InputManager::movingLeft;
bool InputManager::movingRight;
bool InputManager::jumping;

void InputManager::moveForwardDown() {
    movingForward = true;
    sendPlayerInputEvent();
}

void InputManager::moveForwardUp() {
    movingForward = false;
    sendPlayerInputEvent();
}

void InputManager::moveBackwardDown() {
    movingBackward = true;
    sendPlayerInputEvent();
}

void InputManager::moveBackwardUp() {
    movingBackward = false;
    sendPlayerInputEvent();
}

void InputManager::moveLeftDown() {
    movingLeft = true;
    sendPlayerInputEvent();
}

void InputManager::moveLeftUp() {
    movingLeft = false;
    sendPlayerInputEvent();
}

void InputManager::moveRightDown() {
    movingRight = true;
    sendPlayerInputEvent();
}

void InputManager::moveRightUp() {
    movingRight = false;
    sendPlayerInputEvent();
}

void InputManager::jumpDown() {
    jumping = true;
    sendPlayerInputEvent();
}

void InputManager::jumpUp() {
    jumping = false;
    sendPlayerInputEvent();
}

void InputManager::look(int deltaX, int deltaY) {

}

void InputManager::sendPlayerInputEvent() {
    PlayerInputEvent evt(0,
        movingForward,
        movingBackward,
        movingLeft,
        movingRight,
        jumping,
        false,
        0.0f,
        0.0f);

    g_client->sendMessage(evt);
}

InputManager::InputManager(osgViewer::Viewer *viewer) {
    keyboardHandler = new KeyboardEventHandler();
    viewer->addEventHandler(keyboardHandler);
}

InputManager::~InputManager() {
}

void InputManager::loadConfig() {
    keyboardHandler->bindKey('w', KeyboardEventHandler::KEY_DOWN, InputManager::moveForwardDown);
    keyboardHandler->bindKey('w', KeyboardEventHandler::KEY_UP, InputManager::moveForwardUp);
    keyboardHandler->bindKey('s', KeyboardEventHandler::KEY_DOWN, InputManager::moveBackwardDown);
    keyboardHandler->bindKey('s', KeyboardEventHandler::KEY_UP, InputManager::moveBackwardUp);
    keyboardHandler->bindKey('a', KeyboardEventHandler::KEY_DOWN, InputManager::moveLeftDown);
    keyboardHandler->bindKey('a', KeyboardEventHandler::KEY_UP, InputManager::moveLeftUp);
    keyboardHandler->bindKey('d', KeyboardEventHandler::KEY_DOWN, InputManager::moveRightDown);
    keyboardHandler->bindKey('d', KeyboardEventHandler::KEY_UP, InputManager::moveRightUp);
    keyboardHandler->bindKey(' ', KeyboardEventHandler::KEY_DOWN, InputManager::jumpDown);
    keyboardHandler->bindKey(' ', KeyboardEventHandler::KEY_UP, InputManager::jumpUp);
}
