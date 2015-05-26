#include "PlayerInputEventHandler.h"

#include <components/PlayerStatusComponent.h>
#include <components/PositionComponent.h>
#include <components/RotationComponent.h>
#include <core/Entity.h>
#include <core/Player.h>
#include <network/PlayerInputEvent.h>

#include "../components/CharacterRotationComponent.h"
#include "../components/InputComponent.h"
#include "../core/Game.h"
#include "../math/util.h"

PlayerInputEventHandler::PlayerInputEventHandler(Game *game)
        : game(game) {
}

void PlayerInputEventHandler::handle(const Event &e) const {
    const PlayerInputEvent &evt = static_cast<const PlayerInputEvent &>(e);

    Entity *entity = game->getPlayers().at(evt.getPlayerId())->getEntity();

    auto inputComp = entity->getComponent<InputComponent>();

    if (inputComp != nullptr) {
        inputComp->setMovingForward(evt.isMovingForward());
        inputComp->setMovingBackward(evt.isMovingBackward());
        inputComp->setMovingLeft(evt.isMovingLeft());
        inputComp->setMovingRight(evt.isMovingRight());
        inputComp->setJumping(evt.isJumping());
        inputComp->setAttacking1(evt.isAttacking1());
        inputComp->setAttacking2(evt.isAttacking2());
    }

    auto playerStatusComp = entity->getComponent<PlayerStatusComponent>();

    if (playerStatusComp != nullptr) {
        const bool running = evt.isMovingForward() ||
                evt.isMovingBackward() ||
                evt.isMovingLeft() ||
                evt.isMovingRight();
        const bool attacking = evt.isAttacking1() || evt.isAttacking2();

        playerStatusComp->setRunning(running);
        playerStatusComp->setJumping(evt.isJumping());
        playerStatusComp->setAttacking(attacking);
    }

    auto rotComp = entity->getComponent<RotationComponent>();
    rotComp->setRotation(euler2Quat(evt.getYaw(), 0.0f, 0.0f));

    auto charRotComp = entity->getComponent<CharacterRotationComponent>();
    charRotComp->setRotation(euler2Quat(evt.getYaw(), evt.getPitch(), 0.0f));
}
