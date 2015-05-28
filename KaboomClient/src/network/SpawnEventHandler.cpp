#include "SpawnEventHandler.h"

#include <core/EntityType.h>
#include <network/SpawnEvent.h>
#include <components/PositionComponent.h>
#include <ParticleEffectManager.h>
#include <TrailingEffect.h>

#include "../core/Game.h"
#include "../components/SceneNodeComponent.h"
#include "../components/TrailingEffectComponent.h"
#include <GeometryObject.h>

SpawnEventHandler::SpawnEventHandler(Game *game)
        : game(game) {
}

void SpawnEventHandler::handle(const Event &e) const {
    const SpawnEvent &evt = static_cast<const SpawnEvent &>(e);
	EntityManager &entityManager = game->getEntityManager();
    EntityType type = evt.getType();
    Entity *entity = nullptr;
	
	
    if ((type & CAT_MASK) == CAT_CHARACTER) {
        entity = game->getCharacterFactory().createCharacter(
                evt.getEntityId(),
                evt.getType(),
                evt.getPosition(),
                evt.getRotation());
    } else if (evt.isPickup()) { //If it's a pickup we want to create a pickup instead
		entity = game->getPickupFactory().createPickup(
			evt.getEntityId(),
			evt.getType(),
			evt.getPosition(),
			evt.getRotation());
	} else if ((type & CAT_MASK) == CAT_BOMB) {
        entity = game->getBombFactory().createBomb(
                evt.getEntityId(),
                evt.getType(),
                evt.getPosition(),
                evt.getRotation());

        Entity *player = game->getCurrentPlayer()->getEntity();
		if (player != nullptr){


			PositionComponent *playerPosComp = player->getComponent<PositionComponent>();
			PositionComponent *entityPosComp = entity->getComponent<PositionComponent>();
            const Vec3 &playerPos = playerPosComp->getPosition();
            const Vec3 &entityPos = entityPosComp->getPosition();

            double x = (double)(-playerPos.x + entityPos.x);
            double y = (double)(-playerPos.y + entityPos.y);
            double z = (double)(-playerPos.z + entityPos.z);

			switch (type) {
                case KABOOM_V2: {
                    game->getSoundManager().playSound(SoundType::KABOOM_FIRE);
                    break;
                }
			}
		}
	} else if ((type & CAT_MASK) == CAT_JUMPPAD) {
		entity = game->getJumpPadFactory().createJumpPad(
			evt.getEntityId(),
			evt.getType(),
			evt.getPosition(),
			evt.getRotation());
	}

    if (entity != nullptr) {
		game->addEntity(entity);

		if ((entity->getType() & CAT_MASK) == CAT_BOMB) {
			TrailingEffect *effect = static_cast<TrailingEffect *>(
				game->getParticleEffectManager()->getParticleEffect(ParticleEffectManager::TRAILING));

			GeometryObject *geomObj = game->getGeometryManager()->getGeometryObject(std::to_string(entity->getId()));
			effect->setTrailedObject(geomObj);
			effect->run(geomObj->getTranslate());

			entity->attachComponent(new TrailingEffectComponent(effect));
		}
    }
}
