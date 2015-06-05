#include "PlayerStatusEventHandler.h"

#include <components/PlayerStatusComponent.h>
#include "../components/ModelComponent.h"
#include "../components/SoundComponent.h"
#include <components/PositionComponent.h>
#include <components/RotationComponent.h>
#include <core/Entity.h>
#include <network/PlayerStatusEvent.h>

#include "../core/Game.h"
#include <Core.h>			// For animation type-id constants

PlayerStatusEventHandler::PlayerStatusEventHandler(Game *game)
        : game(game) {
}

void PlayerStatusEventHandler::handle(const Event &e) const {
    const PlayerStatusEvent &evt = static_cast<const PlayerStatusEvent &>(e);
    Entity *entity = game->getEntityManager().getEntity(evt.getEntityId());
	if (entity == nullptr){
		return;
	}
    auto playerStatusComp = entity->getComponent<PlayerStatusComponent>();
    playerStatusComp->setAlive(evt.isAlive());
    playerStatusComp->setRunning(evt.isRunning());
    playerStatusComp->setJumping(evt.isJumping());
    playerStatusComp->setAttacking(evt.isAttacking());
    playerStatusComp->setDamaged(evt.isDamaged());
	EntityType type = entity->getType();
	Entity *player = game->getCurrentPlayer()->getEntity();
	auto bombPos = entity->getComponent<PositionComponent>()->getPosition();
	auto playerRot = player->getComponent<RotationComponent>()->getRotation();
	auto playerPos = player->getComponent<PositionComponent>()->getPosition();
	//std::cout << playerPos.x << "  " << playerPos.y << "  " << playerPos.z << std::endl; //remove later
	game->getSoundManager().setListenerPosition(playerPos);
	game->getSoundManager().setListenerRotation(playerRot);
	auto soundComp = entity->getComponent<SoundComponent>();
	if (evt.isRunning()&&!evt.isJumping()){
		soundComp->setListenerPosition(playerPos);
		soundComp->setListenerRotation(playerRot);
		soundComp->playSound(bombPos);
	}
	else{
		soundComp->stopSound();
	}
	if (evt.isJumping()&&!playerStatusComp->getJumped()){
		game->getSoundManager().playSound(SoundType::JUMP, bombPos);
	}
	
	playerStatusComp->setJumped(evt.isJumping());
	
	auto modelComp = entity->getComponent<ModelComponent>();
	Model* model = modelComp->getModel();

	if (evt.isRunning()) {
		model->playAnimation(RUNNING);
	}
	else {
		model->playAnimation(IDLE);
	}
}
