#include "BombFactory.h"

#include <Core.h>
#include <GeometryCache.h>
#include <GeometryObject.h>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Shape>
#include <osg/ShapeDrawable>

#include <components/PositionComponent.h>
#include <components/RotationComponent.h>
#include <core/EntityManager.h>
#include <osgDB/ReadFile>

#include "EntityConfigLookup.h"
#include "../components/SceneNodeComponent.h"

BombFactory::BombFactory(EntityManager &entityManager)
        : entityManager(entityManager) {
	srand(11171992);
}

Entity *BombFactory::createBomb(
        unsigned int id,
        EntityType type,
        const Vec3 &position,
        Quat rotation) const {
    Entity *entity = entityManager.createEntity(id, type);

    //createBase(entity, position, rotation);

	entity->attachComponent(new PositionComponent(position));
	entity->attachComponent(new RotationComponent(rotation));

	if (type == SALTY_MARTY_BOMB)
	{
		auto &config = EntityConfigLookup::get(entity->getType());

		osg::ref_ptr<osg::Sphere> sphere = new osg::Sphere();
		sphere->setRadius(config.getFloat("size"));
		osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(sphere);
		osg::ref_ptr<osg::Geode> model = new osg::Geode;
		model->addDrawable(drawable);

		osg::ref_ptr<osg::MatrixTransform> transformation = new osg::MatrixTransform;
		transformation->addChild(model);

		osg::ref_ptr<osg::Group> bombNode = new osg::Group;

		bombNode->addChild(transformation);

		entity->attachComponent(new SceneNodeComponent(bombNode));
	}
	else if (type == FAKE_BOMB)
	{
		GeometryCache *geoCache = Core::getWorldRef().getGeometryCache();
		osg::ref_ptr<osg::Node> node;
		switch(rand()%3)
		{
			case 0:
				node = geoCache->getNodeById(KABOOM_V2);
				break;
			case 1:
				node = geoCache->getNodeById(TIME_BOMB);
				break;
			case 2:
				node = geoCache->getNodeById(REMOTE_DETONATOR);
				break;
			default:
				node = geoCache->getNodeById(KABOOM_V2);
				break;
		}
		entity->attachComponent(new SceneNodeComponent(node));
	}
	else{
		GeometryCache *geoCache = Core::getWorldRef().getGeometryCache();

		//Material * mat = geoCache->getMaterialById(type);
		osg::ref_ptr<osg::Node> node = geoCache->getNodeById(type);
		entity->attachComponent(new SceneNodeComponent(node));
	}

    return entity;
}