
#include "Model.h"
#include "Core.h"
#include "GeometryCache.h"
#include "GeometryObject.h"
#include <osgDB/ReadFile>

Model::Model() 
{
	_root = new osg::MatrixTransform();
	_curr_type_id = -1;
}

Model::Model(int type_id, bool hasAnim, osgAnimation::Animation::PlayMode playMode)
	: Model()
{
	GeometryCache* cache = Core::getWorldRef().getGeometryCache();

	if (!hasAnim) {
		loadModel(type_id);

		osg::ref_ptr<osg::MatrixTransform> transformNode = dynamic_cast<osg::MatrixTransform*>
			(_root->getChild(0));
		//_root
		transformNode->setMatrix(cache->getMatrixById(type_id));
	}
	else {
		addAnimationById(type_id, playMode);
		std::unordered_map<int, osg::ref_ptr<osg::MatrixTransform>>::iterator itr = _modelMap.find(type_id);
		if (itr == _modelMap.end())
		{
			OSG_WARN << "Model(int type_id, bool hasAnim) should not go here!" << std::endl;
			return;
		}

		_root->addChild(itr->second);
		//_root = itr->second;

		std::unordered_map<int, osg::ref_ptr<osgAnimation::BasicAnimationManager>>::iterator itr2 =
			_animManagerMap.find(type_id);
		if (itr2 == _animManagerMap.end())
		{
			OSG_WARN << "Model::playAnimation(int, std::string&) No manager" << std::endl;
			return;
		}

		_animManager = itr2->second;
	}
}

Model::Model(std::string& modelName)
	: Model()
{
	loadModel(modelName);
}

bool Model::loadModel(int type_id) 
{
	_curr_type_id = type_id;

	GeometryCache* cache = Core::getWorldRef().getGeometryCache();
	osg::ref_ptr<osg::Node> tmp = cache->getNodeById(type_id);
	osg::ref_ptr<osg::MatrixTransform> transformNode = NULL;
	bool retVal = loadModelHelper(tmp, transformNode, _animManager);

	// Use GeometryObject to handle the Material, model will be a grandchild
	// [TODO: Fix this memory leak]
	GeometryObject geom = GeometryObject("Character Dummy", transformNode.get());
	geom.setMaterial(cache->getMaterialById(type_id));
	transformNode = geom.getRoot();

	_root->addChild(transformNode);
	return retVal;
}

bool Model::loadModel(std::string& modelName) 
{
	// Make sure model is loaded
	osg::ref_ptr<osg::Node> tmp = Core::getWorldRef().getGeometryCache()->getNodeByFileName(modelName);
	osg::ref_ptr<osg::MatrixTransform> transformNode = NULL;
	bool retVal = loadModelHelper(tmp, transformNode, _animManager);

	_root->addChild(transformNode);
	return retVal;
}

bool Model::loadModelHelper(osg::ref_ptr<osg::Node> origNode, osg::ref_ptr<osg::Group> newNode, 
	osg::ref_ptr<osgAnimation::BasicAnimationManager>& animManager)
{
	osg::ref_ptr<osg::Node> model = dynamic_cast<osg::Node*>(origNode->clone(osg::CopyOp::DEEP_COPY_ALL));

	if (!model) {
		printf("<Model::loadAnimations> model is null\n");
		return false;
	}

	// Try getting the animation manager from the update callback of the model root
	animManager = dynamic_cast<osgAnimation::BasicAnimationManager*>
		(model->getUpdateCallback());
	if (!animManager) {
		newNode->setUpdateCallback(animManager.get());
	}

	// Create a node that holds the model with the animation manager
	newNode->addChild(model.get());

	return true;
}

osg::ref_ptr<osg::MatrixTransform> Model::getRootNode() 
{
	return _root;
}

void Model::addAnimationById(int type_id, osgAnimation::Animation::PlayMode playMode)
{
	osg::ref_ptr<osg::MatrixTransform> node = new osg::MatrixTransform();
	osg::ref_ptr<osgAnimation::BasicAnimationManager> animManager = NULL;

	GeometryCache* cache = Core::getWorldRef().getGeometryCache();
	osg::ref_ptr<osg::Node> tmp = cache->getNodeById(type_id);

	loadModelHelper(tmp, node, animManager);

	// Set the animation play mode (Default is loop)
	animManager->getAnimationList()[0]->setPlayMode(playMode);

	// Use GeometryObject to handle the Material, model will be a grandchild
	// [TODO: Fix this memory leak]
	GeometryObject* geom = new GeometryObject("Character Dummy" + type_id, node.get());
	geom->setMaterial(cache->getMaterialById(type_id));
	node = geom->getRoot();

	node->setMatrix(cache->getMatrixById(type_id));

	_modelMap.insert(std::make_pair(type_id, node));
	_animManagerMap.insert(std::make_pair(type_id, animManager));
}

void Model::playAnimation(std::string& animName) {
	if (!this->_animManager) {
		printf("<Model::playAnimation> animManager is null\n");
	}

	// Iterate over all animations recorded in the manager
	const osgAnimation::AnimationList& animations =
		_animManager->getAnimationList();

	// Find the animation to play
	for (unsigned int i = 0; i<animations.size(); ++i)
	{
		const std::string& name = animations[i]->getName();
		//std::cout << "name:" << name << std::endl;
		if (name == animName) {
			_animManager->playAnimation(animations[i].get());
			break;
		}
	}
}

void Model::playAnimation(int type_id, std::string& animName)
{
	// Switch models if using animation from a different one
	if (_curr_type_id != type_id) {

		// Stop any previous animations
		if (_curr_type_id != -1) {
			stopAnimation();
		}

		std::unordered_map<int, osg::ref_ptr<osg::MatrixTransform>>::iterator itr1 = _modelMap.find(type_id);
		if (itr1 == _modelMap.end())
		{
			OSG_WARN << "Model::playAnimation(int, std::string&) No model" << std::endl;
			return;
		}

		_root->setChild(0, itr1->second);
		//_root = itr1->second;

		// Set current animation manager to the playing animation
		std::unordered_map<int, osg::ref_ptr<osgAnimation::BasicAnimationManager>>::iterator itr2 =
			_animManagerMap.find(type_id);
		if (itr2 == _animManagerMap.end())
		{
			OSG_WARN << "Model::playAnimation(int, std::string&) No manager" << std::endl;
			return;
		}

		_animManager = itr2->second;
		_curr_type_id = type_id;

		playAnimation(animName);
	}
}

void Model::stopAnimation() 
{
	if (!this->_animManager) {
		printf("<Model::stopAnimation> animManager is null\n");
	}
	_animManager->stopAll();
	//manager->stopAnimation();
}
