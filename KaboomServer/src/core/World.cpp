#include "World.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <core/Entity.h>
#include <core/EntityType.h>

#include <osgbDynamics/MotionState.h>
#include <osgbCollision/CollisionShapes.h>


#include "../core/OsgObjectConfigLoader.h"
#include "../components/CollisionComponent.h"
#include "../components/TriggerComponent.h"

void onTickCallback(btDynamicsWorld *world, btScalar timeStep) {
    World *w = static_cast<World *>(world->getWorldUserInfo());
    w->onTick(timeStep);
}

World::World(ConfigSettings * configSettings)
        : dispatcher(&collisionConfiguration),
          world(&dispatcher, &broadphase, &solver, &collisionConfiguration),
		  config(configSettings){

	config->getValue(ConfigSettings::str_mediaFilePath, mediaPath);

    setGravity(4.0f);
    broadphase.getOverlappingPairCache()->setInternalGhostPairCallback(new TriggerCallback());
    world.setInternalTickCallback(onTickCallback, this);

	//TODO make an on off switch here to disable debugViewer
	debugViewer = new OsgBulletDebugViewer(config);
	debugViewer->init();
	world.setDebugDrawer(debugViewer->getDbgDraw());
}

World::~World() {
}

void World::loadMap() {

    addStaticPlane(btVector3(0, 0, 0), btVector3(0, 0, 1));//floor
    addStaticPlane(btVector3(0, -10, 0), btVector3(0, 1, 0));//back wall
    addStaticPlane(btVector3(0, 10, 0), btVector3(0, -1, 0));//front wall
    addStaticPlane(btVector3(-10, 0, 0), btVector3(1, 0, 0));//left wall
    addStaticPlane(btVector3(10, 0, 0), btVector3(-1, 0, 0));//right wall

    //add a ramp

    btQuaternion quat;
    quat.setRotation(btVector3(0, 1, 0), btRadians(btScalar(30)));
    addStaticPlane(btVector3(-5, 0, 0), btVector3(0, 0, 1), quat);
}

void World::loadMapFromXML(const std::string &mapXMLFile){

	OsgObjectConfigLoader osgObjectConfigLoader(osgNodeConfigMap);
	osgObjectConfigLoader.load(mapXMLFile);

	for (auto it = osgNodeConfigMap.begin(); it != osgNodeConfigMap.end(); ++it) {
		Configuration osgObjectConfig = it->second;

		std::string filePath= osgObjectConfig.getString("file");
		osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(mediaPath + filePath);
		osg::ref_ptr<osg::MatrixTransform> transfromNode = new osg::MatrixTransform;
		transfromNode->addChild(node);

		osg::Matrix mat;

		osg::Vec3 pos, scale;
		osg::Quat rot, so;
		mat.decompose(pos, rot, scale, so);

		mat.makeTranslate(osgObjectConfig.getVec3("position"));
		mat.preMult(osg::Matrix::rotate(osg::Quat(osgObjectConfig.getVec4("orientation"))));
		mat.preMult(osg::Matrix::scale(osgObjectConfig.getVec3("scale")));

		transfromNode->setMatrix(mat);

		osgbDynamics::MotionState * motion = new osgbDynamics::MotionState;
		motion->setTransform(transfromNode);

		btCollisionShape * collisionShape = osgbCollision::btTriMeshCollisionShapeFromOSG(transfromNode);
		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motion, collisionShape, btVector3(0, 0, 0));
		btRigidBody * rigidbody = new btRigidBody(rigidBodyCI);
		addRigidBody(rigidbody);
		
		debugViewer->addNode(transfromNode);



		/*osg::MatrixTransform* node = osgObjectConfig.getPointer<osg::MatrixTransform *>("node");

		osgbDynamics::MotionState * motion = new osgbDynamics::MotionState;
		motion->setTransform(node);

		btCollisionShape * collisionShape = osgbCollision::btTriMeshCollisionShapeFromOSG(node);

		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motion, collisionShape, btVector3(0, 0, 0));
		btRigidBody * rigidbody = new btRigidBody(rigidBodyCI);
		addRigidBody(rigidbody);

		debugViewer->addNode(node);*/
		//osg::ref_ptr<osg::Node> debugNode = osgbCollision::osgNodeFromBtCollisionShape(rigidbody->getCollisionShape());//osgbCollision::osgNodeFromBtCollisionShape(rigidBody->getCollisionShape());
		//debugViewer->addNode(debugNode);
		//TODO what happen then to the MatrixTransform????

		//osg::Node* debugNode = osgbCollision::osgNodeFromBtCollisionShape(collisionShape);
		//debugViewer->addNodeWireFrame(debugNode);


		// Create an OSG representation of the Bullet shape and attach it.
		// This is mainly for debugging.
		//osg::Node* debugNode = osgbCollision::osgNodeFromBtCollisionShape(collision);
		//node->addChild(debugNode);


		// Set debug node state.
		//osg::StateSet* state = debugNode->getOrCreateStateSet();
		//osg::PolygonMode* pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
		//state->setAttributeAndModes(pm);
		//osg::PolygonOffset* po = new osg::PolygonOffset(-1, -1);
		//state->setAttributeAndModes(po);
		//state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	}
}

void World::stepSimulation(float timeStep, int maxSubSteps) {

	debugViewer->getDbgDraw()->BeginDraw();

    world.stepSimulation(timeStep, maxSubSteps);

	world.debugDrawWorld();
	debugViewer->getDbgDraw()->EndDraw();
	
}

void World::addRigidBody(btRigidBody *rigidBody) {
	world.addRigidBody(rigidBody);
}

void World::addRigidBodyAndConvertToOSG(btRigidBody *rigidBody){
	osg::ref_ptr<osg::Node> node = osgbCollision::osgNodeFromBtCollisionShape(rigidBody->getCollisionShape());
	debugViewer->addNode(node);

	world.addRigidBody(rigidBody);
}
void World::removeRigidBody(btRigidBody *rigidBody) {
    world.removeRigidBody(rigidBody);
}

void World::addTrigger(btGhostObject *ghostObject) {
    ghostObject->setCollisionFlags(ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    world.addCollisionObject(
            ghostObject,
            btBroadphaseProxy::SensorTrigger,
            btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::SensorTrigger ^ btBroadphaseProxy::StaticFilter);
}

void World::removeTrigger(btGhostObject *ghostObject) {
    world.removeCollisionObject(ghostObject);
}

void World::setGravity(float gravity) {
    world.setGravity(btVector3(0, 0, -gravity));
}

void World::onTick(btScalar timeStep) {
    int numManifolds = dispatcher.getNumManifolds();

    for (int i = 0; i < numManifolds; ++i) {
        const btPersistentManifold *manifold = dispatcher.getManifoldByIndexInternal(i);

        int numContacts = manifold->getNumContacts();

        if (numContacts == 0) {
            continue;
        }

        const btCollisionObject *collisionObjA = static_cast<const btCollisionObject *>(manifold->getBody0());
        const btCollisionObject *collisionObjB = static_cast<const btCollisionObject *>(manifold->getBody1());

        // Ignore ghost objects.
        if (!collisionObjA->hasContactResponse() || !collisionObjB->hasContactResponse()) {
            continue;
        }

        Entity *entityA = static_cast<Entity *>(collisionObjA->getUserPointer());
        Entity *entityB = static_cast<Entity *>(collisionObjB->getUserPointer());

        handleCollision(entityA, entityB);
        handleCollision(entityB, entityA);
    }
}

const btCollisionDispatcher &World::getDispatcher() const {
    return dispatcher;
}

void World::addStaticPlane(btVector3 origin, btVector3 normal) {
    addStaticPlane(origin, normal, btQuaternion::getIdentity());
}

void World::addStaticPlane(btVector3 origin, btVector3 normal, btQuaternion rotation) {
    btTransform startTrans;
    startTrans.setIdentity();
    startTrans.setOrigin(origin);
    startTrans.setRotation(rotation);

    btCollisionShape *groundShape = new btStaticPlaneShape(normal, 0);
    btDefaultMotionState *groundMotionState = new btDefaultMotionState(startTrans);

    //mass, motionshape, collisionShape, localInertia
    //mass = 0, means static objects!
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody *groundRigidBody = new btRigidBody(groundRigidBodyCI);

    addRigidBody(groundRigidBody);
}

void World::handleCollision(Entity *entityA, Entity *entityB) const {
    if (entityA != nullptr) {
        CollisionComponent *colComp = entityA->getComponent<CollisionComponent>();

        if (colComp != nullptr) {
            colComp->setCollided(true);

            if (entityB != nullptr) {
                colComp->addContactEntity(entityB);
            }
        }
    }
}

void World::renderDebugFrame() {

	debugViewer->renderFrame();

}

OsgBulletDebugViewer * World::getDebugViewer(){
	return debugViewer;
}

void World::debugDrawWorld() {
	world.debugDrawWorld();
}

btBroadphasePair *World::TriggerCallback::addOverlappingPair(btBroadphaseProxy *proxy0, btBroadphaseProxy *proxy1) {
    btCollisionObject *colObj0 = static_cast<btCollisionObject *>(proxy0->m_clientObject);
    btCollisionObject *colObj1 = static_cast<btCollisionObject *>(proxy1->m_clientObject);

    Entity *entity0 = static_cast<Entity *>(colObj0->getUserPointer());
    Entity *entity1 = static_cast<Entity *>(colObj1->getUserPointer());

    addTriggerEntity(entity0, entity1);
    addTriggerEntity(entity1, entity0);

    return btGhostPairCallback::addOverlappingPair(proxy0, proxy1);
}



void *World::TriggerCallback::removeOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1, btDispatcher* dispatcher) {
    btCollisionObject *colObj0 = static_cast<btCollisionObject *>(proxy0->m_clientObject);
    btCollisionObject *colObj1 = static_cast<btCollisionObject *>(proxy1->m_clientObject);

    Entity *entity0 = static_cast<Entity *>(colObj0->getUserPointer());
    Entity *entity1 = static_cast<Entity *>(colObj1->getUserPointer());

    removeTriggerEntity(entity0, entity1);
    removeTriggerEntity(entity1, entity0);

    return btGhostPairCallback::removeOverlappingPair(proxy0, proxy1, dispatcher);
}

void World::TriggerCallback::addTriggerEntity(Entity *entityA, Entity *entityB) const {
    if (entityA != nullptr) {
        TriggerComponent *triggerComp = entityA->getComponent<TriggerComponent>();

        if (triggerComp != nullptr && entityB != nullptr) {
            triggerComp->addTriggerEntity(entityB);
        }
    }
}

void World::TriggerCallback::removeTriggerEntity(Entity *entityA, Entity *entityB) const {
    if (entityA != nullptr) {
        TriggerComponent *triggerComp = entityA->getComponent<TriggerComponent>();

        if (triggerComp != nullptr && entityB != nullptr) {
            triggerComp->removeTriggerEntity(entityB);
        }
    }
}
