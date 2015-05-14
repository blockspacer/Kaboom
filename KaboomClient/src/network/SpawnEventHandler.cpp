#include "SpawnEventHandler.h"

#include <core/EntityType.h>
#include <network/SpawnEvent.h>
#include <components/PositionComponent.h>
#include "../core/Game.h"

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
                evt.getX(),
                evt.getY(),
                evt.getZ(),
                evt.getRotation());
    } else if ((type & CAT_MASK) == CAT_BOMB) {
        entity = game->getBombFactory().createBomb(
                evt.getEntityId(),
                evt.getType(),
                evt.getX(),
                evt.getY(),
                evt.getZ(),
                evt.getRotation());
		game->source = new Source;
		Entity *player = entityManager.getEntity(game->getGameClient().getCurrentPlayerEntityId());
		if (player != nullptr){


			PositionComponent *playerPos = player->getComponent<PositionComponent>();
			PositionComponent *pos = entity->getComponent<PositionComponent>();
			double x = (double)(-playerPos->getX() + pos->getX());
			double y = (double)(-playerPos->getY() + pos->getY());
			double z = (double)(-playerPos->getZ() + pos->getZ());

			switch (type&(~CAT_BOMB)){
			case KABOOM_V2:
				game->source->setSound(game->sounds->at(KABOOM_FIRE).get());
				break;
			default:
				game->source->setSound(game->sounds->at(BASIC).get());
				printf("unknown bomb type gets no sound\n");
				break;
			}
			game->source->setRolloffFactor(sqrt(x*x + y*y + z*z));
			game->source->setGain(1);
			game->source->setLooping(false);
			osgAudio::AudioEnvironment::instance()->update();
			game->source->play();
		}
    }

    if (entity != nullptr) {
        game->addEntity(entity);
    }
}
