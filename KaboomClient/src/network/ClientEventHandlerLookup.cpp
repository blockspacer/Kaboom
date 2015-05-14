#include "ClientEventHandlerLookup.h"

#include "DestroyEventHandler.h"
#include "DisconnectEventHandler.h"
#include "ExplosionEventHandler.h"
#include "HealthEventHandler.h"
#include "PositionEventHandler.h"
#include "RotationEventHandler.h"
#include "ScoreEventHandler.h"
#include "SpawnEventHandler.h"

ClientEventHandlerLookup::ClientEventHandlerLookup(Game *game) {
    addHandler(EVENT_DISCONNECT, new DisconnectEventHandler(game));
    addHandler(EVENT_SPAWN, new SpawnEventHandler(game));
    addHandler(EVENT_DESTROY, new DestroyEventHandler(game));
    addHandler(EVENT_POSITION, new PositionEventHandler(game));
    addHandler(EVENT_ROTATION, new RotationEventHandler(game));
    addHandler(EVENT_EXPLOSION, new ExplosionEventHandler(game));
    addHandler(EVENT_HEALTH, new HealthEventHandler(game));
    addHandler(EVENT_SCORE, new ScoreEventHandler(game));
}

ClientEventHandlerLookup::~ClientEventHandlerLookup() {
}
