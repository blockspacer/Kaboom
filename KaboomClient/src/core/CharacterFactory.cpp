#include "CharacterFactory.h"

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Shape>
#include <osg/ShapeDrawable>

#include <core/EntityManager.h>
#include <core/PositionComponent.h>
#include <core/RotationComponent.h>

#include "SceneNodeComponent.h"

CharacterFactory::CharacterFactory(EntityManager &entityManager)
        : entityManager(entityManager) {
}

Entity *CharacterFactory::createCharacter(
        unsigned int id,
        EntityType characterType,
        float x,
        float y,
        float z,
        float yaw,
        float pitch) const {
    Entity *entity = entityManager.createEntity(id, characterType);

    osg::ref_ptr<osg::Box> box = new osg::Box;
    osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(box);
    osg::ref_ptr<osg::Geode> model = new osg::Geode;
    model->addDrawable(drawable);

    osg::ref_ptr<osg::MatrixTransform> transformation = new osg::MatrixTransform;
    transformation->addChild(model);

    osg::ref_ptr<osg::Group> playerNode = new osg::Group;

    playerNode->addChild(transformation);

    entity->attachComponent(new SceneNodeComponent(playerNode));
    entity->attachComponent(new PositionComponent(x, y, z));
    entity->attachComponent(new RotationComponent(yaw, pitch));

    return entity;
}
