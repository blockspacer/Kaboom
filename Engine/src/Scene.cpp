#include "stdafx.h" 

#include "Scene.h"
#include "World.h"
#include "Core.h"
#include "GeometryObjectManager.h"
#include "LightManager.h"
#include "MaterialManager.h"

#include <osgDB/ReadFile>

void setupScene()
{
	World &w = Core::getWorldRef();
	GeometryObjectManager *g = w.getGeometryManager();
	MaterialManager *m = w.getMaterialManager();
	LightManager *l = w.getLightManager();

	std::string base = "Cerberus_by_Andrew_Maximov/Textures/";
	osg::Node *loaded = osgDB::readNodeFile("Cerberus_by_Andrew_Maximov/Cerberus_LP.fbx");
	g->addGeometry("gun", loaded);

	m->createTextureMaterial("gunMaterial",
		base + "Cerberus_A.tga",
		base + "Cerberus_R.tga",
		base + "Cerberus_A.tga",
		base + "Cerberus_M.tga",
		base + "Cerberus_N.tga");

	g->setGeometryMaterial("gun", m->getMaterial("gunMaterial"));

	 osg::Vec3 skyColor = osg::Vec3(0.196078, 0.6, 0.8);
	osg::Vec3 sunsetColor = osg::Vec3(0.9, 0.8, 0.5);
	l->addDirectionalLight("Sun", osg::Vec3(-1, -1, -1), sunsetColor, false);

	Core::setEnvironmentMap("Cubemap_snow/posx.jpg", "Cubemap_snow/negx.jpg",
		"Cubemap_snow/posy.jpg", "Cubemap_snow/negy.jpg",
		"Cubemap_snow/posz.jpg", "Cubemap_snow/negz.jpg");

	osg::Node *loadedCow = osgDB::readNodeFile("cow.osg");

	g->addGeometry("cow", loadedCow);
	m->createPlainMaterial("planeMat", osg::Vec3(1.0, 1.0, 1.0), 0.5, 0.7, 0.3);
	g->setGeometryMaterial("cow", m->getMaterial("planeMat"));
	//l->addDirectionalLight("Sun", osg::Vec3(-1, -1, -1), osg::Vec3(0.7, 0.3, 0.3), false);

	osg::Vec3 colorRED = osg::Vec3(0.7, 0.3, 0.3);
	l->addPointLight("pt1", osg::Vec3(7, 0, 0), colorRED, 100, false);

	//osg::Vec3 colorGREEN = osg::Vec3(0.3, 0.7, 0.3);
	//l->addPointLight("pt2", osg::Vec3(-7, 0, 0), colorGREEN, 100, false);

	//osg::Vec3 colorYELLO = osg::Vec3(0.7, 0.7, 0.3);
	//l->addPointLight("pt3", osg::Vec3(0, 7, 0), colorYELLO, 100, false);

	//osg::Vec3 colorBLUE = osg::Vec3(0.3, 0.3, 0.7);
	//l->addPointLight("pt4", osg::Vec3(0, -7, 0), colorBLUE, 100, false);

}