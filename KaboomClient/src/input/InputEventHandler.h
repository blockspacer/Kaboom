#pragma once

class GameClient;

class InputEventHandler {
public:
    InputEventHandler(GameClient &client);

    void onMoveForwardDown();
    void onMoveForwardUp();
    void onMoveBackwardDown();
    void onMoveBackwardUp();
    void onMoveLeftDown();
    void onMoveLeftUp();
    void onMoveRightDown();
    void onMoveRightUp();
    void onJumpDown();
    void onJumpUp();
    void onAttack1Down();
    void onAttack1Up();
    void onAttack2Down();
    void onAttack2Up();
    void onLook(float yaw, float pitch);

    // void enterGameMode();
    void quitGameMode();
    // void showDebugAnalysis();
    // void hideDebugAnalysis();

private:
    GameClient &client;

    bool movingForward;
    bool movingBackward;
    bool movingLeft;
    bool movingRight;
    bool jumping;
    bool attacking1;
    bool attacking2;
    float yaw;
    float pitch;

    void sendPlayerInputEvent();
};
