#pragma once

#include <btBulletDynamicsCommon.h>

#include <components/Component.h>

class RespawnComponent : public Component {
public:
	RespawnComponent(int duration)
            : duration(duration) {
    }

	RespawnComponent(int duration, std::string spawnPointName)
            : duration(duration),
              spawnPointName(spawnPointName) {
    }

	RespawnComponent(float x, float y, float z)
            : spawnLocationVec(x, y, z) {
    }

    inline btVector3 getSpawnLocationVec() {
        return spawnLocationVec;
    }

    inline void setSpawnLocationVec(float x, float y, float z) {
        spawnLocationVec.setX(x);
        spawnLocationVec.setY(y);
        spawnLocationVec.setZ(z);
    }

    inline const int getDuration() const {
        return duration;
    }

    inline void setSpawnPointName(std::string spawnPointName) {
        this->spawnPointName = spawnPointName;
    }

    inline const std::string &getSpawnPointName() const {
        return spawnPointName;
    }

private:
    btVector3 spawnLocationVec;
    int duration;
    std::string spawnPointName;
};