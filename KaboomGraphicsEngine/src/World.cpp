#include "World.h"

#include <osgDB/ReadFile>
#include "stdafx.h" 

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Options>
#include <osgDB/XmlParser>

#include "GeometryObject.h"
#include "GeometryObjectManager.h"
#include "LightManager.h"
#include "MaterialManager.h"

World::World()
{
	_geomManager = new GeometryObjectManager;
	_materialManager = new MaterialManager;
	_lightManager = new LightManager;
}

World::~World()
{
	delete _geomManager;
	delete _materialManager;
	delete _lightManager;
}

bool World::isXMLNodeType(osgDB::XmlNode* xmlNode)
{
	switch (xmlNode->type)
	{
	case osgDB::XmlNode::ATOM:
	case osgDB::XmlNode::NODE:
	case osgDB::XmlNode::GROUP:
		return true;
	default:
		return false;
	}
}

void World::loadWorldXMLFile(const std::string &worldFilePath)
{
	osg::ref_ptr<osgDB::XmlNode> xmlRoot = osgDB::readXmlFile(worldFilePath);
	if (xmlRoot.valid())
	{
		osgDB::FilePathList& filePaths = osgDB::getDataFilePathList();
		filePaths.push_back(osgDB::getFilePath(worldFilePath));
		loadXMLNode(xmlRoot.get());
		filePaths.pop_back();
	}
	else
	{
		// TODO : output to console bad xml
	}
}
void World::loadXMLNode(osgDB::XmlNode *xmlRoot)
{
	// Ignore the most outer tag ("world")
	if (xmlRoot->type == osgDB::XmlNode::ROOT)
	{
		for (unsigned int i = 0; i < xmlRoot->children.size(); ++i)
		{
			osgDB::XmlNode* xmlChild = xmlRoot->children[i];
			if (xmlChild->name == "world")
				return loadXMLNode(xmlChild);
		}
	}

	for (unsigned int i = 0; i < xmlRoot->children.size(); ++i)
	{
		osgDB::XmlNode* xmlChild = xmlRoot->children[i];
		if (!isXMLNodeType(xmlChild)) continue;

		const std::string& childName = xmlChild->name;
		//std::cout << childName << std::endl;

		if (childName == "material") {
			std::string type = xmlChild->properties["type"];

			if (type == "plain") {
				createPlainMaterialFromXML(xmlChild);
			}
			else if (type == "textured") {
				createTexturedMaterialFromXML(xmlChild);
			}
		}
		else if (childName == "model") {
			createModelFromXML(xmlChild);
		}
		else if (childName == "light") {
			createLightFromXML(xmlChild);
		}
	}
}

void World::createPlainMaterialFromXML(osgDB::XmlNode* xmlNode)
{
	std::string name = xmlNode->properties["name"];

	osg::Vec3 albedo;
	float roughness = 0.0, specular = 0.0, metallic = 0.0;

	for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
	{
		osgDB::XmlNode* xmlChild = xmlNode->children[i];
		if (!isXMLNodeType(xmlChild)) continue;

		const std::string& childName = xmlChild->name;

		if (childName == "albedo") {
			loadVec3(xmlChild, albedo);
		}
		else if (childName == "roughness") {
			loadFloat(xmlChild, roughness);
		}
		else if (childName == "specular") {
			loadFloat(xmlChild, specular);
		}
		else if (childName == "metallic") {
			loadFloat(xmlChild, metallic);
		}
	}

	_materialManager->createPlainMaterial(name, albedo, roughness, specular, metallic);
}

void World::createTexturedMaterialFromXML(osgDB::XmlNode* xmlNode)
{
	std::string name = xmlNode->properties["name"];

	std::string albedoTex, roughnessTex, metallicTex, normalPath;

	for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
	{
		osgDB::XmlNode* xmlChild = xmlNode->children[i];
		if (!isXMLNodeType(xmlChild)) continue;

		const std::string& childName = xmlChild->name;

		if (childName == "albedoTex") {
			loadString(xmlChild, albedoTex);
		}
		else if (childName == "roughnessTex") {
			loadString(xmlChild, roughnessTex);
		}
		else if (childName == "metallicTex") {
			loadString(xmlChild, metallicTex);
		}
		else if (childName == "normalPath") {
			loadString(xmlChild, normalPath);
		}
	}

	setDefaultString(albedoTex);
	setDefaultString(roughnessTex);
	setDefaultString(metallicTex);
	setDefaultString(normalPath);

	_materialManager->createTextureMaterial(name,
		albedoTex, roughnessTex, metallicTex, normalPath);
}

void World::createModelFromXML(osgDB::XmlNode* xmlNode)
{
	std::string name = xmlNode->properties["name"];
	osg::Node *model = nullptr;
	osg::Vec3 position;
	osg::Vec4 orientation;

	for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
	{
		osgDB::XmlNode* xmlChild = xmlNode->children[i];
		if (!isXMLNodeType(xmlChild)) continue;

		const std::string& childName = xmlChild->name;

		if (childName == "file") {
			std::string file;
			loadString(xmlChild, file);

			model = osgDB::readNodeFile(file);
			_geomManager->addGeometry(name, model, file);
		}
		else if (childName == "material") {
			std::string mat = xmlChild->properties["name"];
			Material *m = _materialManager->getMaterial(mat);

			_geomManager->setGeometryMaterial(name, m);
		}
		else if (childName == "position") {
			loadVec3(xmlChild, position);

			osg::Matrix translate;
			translate.makeTranslate(position);

			GeometryObject* tmp;
			tmp = _geomManager->getGeometryObject(name);
			tmp->setTranslate(position);

		}
		else if (childName == "orientation") {
			loadVec4(xmlChild, orientation);

			osg::Quat quat = osg::Quat(orientation);

			GeometryObject* tmp;
			tmp = _geomManager->getGeometryObject(name);
			tmp->setRotation(orientation);
		}
		//else if (childName == "collider") {
		//	std::stringstream ss; ss << xmlChild->getTrimmedContents();
		//	ss >> metallic;
		//}
	}

}

void World::createLightFromXML(osgDB::XmlNode* xmlNode)
{
	std::string name = xmlNode->properties["name"];
	std::string type = xmlNode->properties["type"];
	osg::Vec3 color, position, direction;
	float radius;
	bool doShadow;

	for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
	{
		osgDB::XmlNode* xmlChild = xmlNode->children[i];
		if (!isXMLNodeType(xmlChild)) continue;

		const std::string& childName = xmlChild->name;

		if (childName == "color") {
			loadVec3(xmlChild, color);
		}
		else if (childName == "position") {
			loadVec3(xmlChild, position);
		}
		else if (childName == "direction") {
			loadVec3(xmlChild, direction);
		}
		else if (childName == "radius") {
			loadFloat(xmlChild, radius);
		}
		else if (childName == "castShadow") {
			loadBool(xmlChild, doShadow);
		}
	}

	if (type == "point") {
		_lightManager->addPointLight(name, position, color, radius, doShadow);
	}
	else if (type == "directional") {
		_lightManager->addDirectionalLight(name, direction, color, doShadow);
	}
}

void World::setDefaultString(std::string &s) {
	s = s.empty() ? "default" : s;
}

void World::loadInt(osgDB::XmlNode* xmlNode, int& i) {
	std::stringstream ss; ss << xmlNode->getTrimmedContents();
	ss >> i;
}

void World::loadFloat(osgDB::XmlNode* xmlNode, float& f) {
	std::stringstream ss; ss << xmlNode->getTrimmedContents();
	ss >> f;
}

void World::loadBool(osgDB::XmlNode* xmlNode, bool& b) {
	std::stringstream ss; ss << xmlNode->getTrimmedContents();
	ss >> b;
}

void World::loadString(osgDB::XmlNode* xmlNode, std::string& s) {
	std::stringstream ss; ss << xmlNode->getTrimmedContents();
	ss >> s;
}

void World::loadVec3(osgDB::XmlNode* xmlNode, osg::Vec3& vec) {
	std::stringstream ss; ss << xmlNode->getTrimmedContents();

	// Read in a vector of floats
	for (int n = 0; n < vec.num_components; ++n)
	{
		float v = 0.0f; ss >> v;
		vec[n] = v;
	}
}

void World::loadVec4(osgDB::XmlNode* xmlNode, osg::Vec4& vec) {
	std::stringstream ss; ss << xmlNode->getTrimmedContents();

	// Read in a vector of floats
	for (int n = 0; n < vec.num_components; ++n)
	{
		float v = 0.0f; ss >> v;
		vec[n] = v;
	}
}
