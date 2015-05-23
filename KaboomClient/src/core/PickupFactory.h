#pragma once

#include <math/Quat.h>

class Entity;
enum EntityType;
class EntityManager;
class Vec3;

class PickupFactory {
public:
	PickupFactory(EntityManager &entityManager);

	void loadPickup(std::string mediaPath);

    Entity *createPickup(
            unsigned int id,
            EntityType type,
            const Vec3 &position,
            Quat rotation) const;

private:
    EntityManager &entityManager;
	osg::ref_ptr<osg::Node> _kaboom2_0_ammo;
	osg::ref_ptr<osg::Node> _timer_ammo;
	osg::ref_ptr<osg::Node> _remote_ammo;
	osg::ref_ptr<osg::Node> _health_pack;

	void createBase(Entity *entity, const Vec3 &position, Quat rotation) const;
	void createKaboomV2Pickup(Entity *entity, const Vec3 &position, Quat rotation) const;
	void createTimeBombPickup(Entity *entity, const Vec3 &position, Quat rotation) const;
	void createRemoteDetonatorPickup(Entity *entity, const Vec3 &position, Quat rotation) const;
	void createHealthPackPickup(Entity *entity, const Vec3 &position, Quat rotation) const;
};
