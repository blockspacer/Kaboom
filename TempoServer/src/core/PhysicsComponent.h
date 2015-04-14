#pragma once

#include <btBulletDynamicsCommon.h>

#include <core/Component.h>

class PhysicsComponent : public Component {
public:
    PhysicsComponent(btRigidBody *rigidBody);
    ~PhysicsComponent();

    btRigidBody *getRigidBody() const;
    void setRigidBody(btRigidBody *rigidBody);

private:
    btRigidBody *rigidBody;
};
