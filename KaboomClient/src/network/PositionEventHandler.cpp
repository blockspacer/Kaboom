#include "PositionEventHandler.h"

#include <osg/MatrixTransform>
#include <Core.h>
#include <GeometryObject.h>
#include <core/Entity.h>
#include <core/PositionComponent.h>
#include <network/PositionEvent.h>

#include "../core/Game.h"
#include "../core/SceneNodeComponent.h"

PositionEventHandler::PositionEventHandler(Game *game)
    : game(game) {
}

PositionEventHandler::~PositionEventHandler() {
}

void PositionEventHandler::handle(const Event &e) const {
    const PositionEvent &evt = static_cast<const PositionEvent &>(e);

    Entity *entity = game->getEntityManager().getEntity(evt.getEntityId());

	// TODO: a hack for not crashing currently . change this to intended behavior.
	if (!entity) return;

    PositionComponent *positionCom = entity->getComponent<PositionComponent>();

    if (positionCom != nullptr) {
        positionCom->setPosition(evt.getX(), evt.getY(), evt.getZ());
    }

	game->getGeometryManager()->getGeometryObject(std::to_string(static_cast<int>(entity->getId())))->setTranslate(osg::Vec3(positionCom->getX(), positionCom->getY(), positionCom->getZ()));

	/*
    SceneNodeComponent *sceneNodeCom = entity->getComponent<SceneNodeComponent>();

    if (sceneNodeCom != nullptr) {
        osg::Group *rootNode = static_cast<osg::Group *>(sceneNodeCom->getNode());
        osg::MatrixTransform *transformNode = static_cast<osg::MatrixTransform *>(rootNode->getChild(0));

        osg::Matrix transformMat;
        transformMat.makeTranslate(evt.getX(), evt.getY(), evt.getZ());

        transformNode->setMatrix(transformMat);
    }
	*/
	
}
