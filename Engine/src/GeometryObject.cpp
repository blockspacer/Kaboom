#include "stdafx.h" 

#include "GeometryObject.h"
#include <osgDB/ReadFile>

#include "Material.h"
#include "MaterialManager.h"
#include "Core.h"

GeometryObject::GeometryObject(const std::string &name, osg::Node *geomNode)
{
	getPlainShader();
	getTexturedShader();

	_objRoot = new osg::MatrixTransform;
	_materialNode = new osg::Group;
	_objRoot->addChild(_materialNode);
	_materialNode->addChild(geomNode);

	_objRoot->setUpdateCallback(new GeometryObjectNodeUpadateCallback(this));

	setMaterial(Core::getWorldRef().getMaterialManager()->getBuiltInMaterial(DEFAULT));
}

GeometryObject::~GeometryObject()
{
}

void GeometryObject::setMaterial(Material *material)
{
	_material = material;

	// reset
	setUpMaterialState();
}

void GeometryObject::setTransform(const osg::Matrixf &transform)
{
	// TODO : test
	_objRoot->setMatrix(transform);
}

osg::Vec3 GeometryObject::getTranslate()
{
	const osg::Matrix &mat = _objRoot->getMatrix();
	return mat.getTrans();
}

void GeometryObject::setTranslate(const osg::Vec3 &translate)
{
	osg::Matrix mat = _objRoot->getMatrix();
	mat.setTrans(translate);
	_objRoot->setMatrix(mat);
}

osg::Quat GeometryObject::getRotation()
{
	osg::Quat rot = _objRoot->getMatrix().getRotate();
	return rot;
}

void GeometryObject::setRotation(const osg::Quat &rot)
{
	osg::Matrix mat = _objRoot->getMatrix();
	mat.setRotate(rot);
	_objRoot->setMatrix(mat);
}

void GeometryObject::setUpMaterialState()
{
	osg::ref_ptr<osg::StateSet> ss = _materialNode->getOrCreateStateSet();

	// for changing materials
	ss->clear();
	
	if (_material->getUseTexture())
	{
		// TODO : implement
	}
	else
	{
		ss->setAttributeAndModes(getPlainShader(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		ss->addUniform(new osg::Uniform("u_albedo", osg::Vec3f(_material->getAlbedo())));
		ss->addUniform(new osg::Uniform("u_roughness", _material->getRoughness()));
		ss->addUniform(new osg::Uniform("u_metallic", _material->getMetallic()));
		ss->addUniform(new osg::Uniform("u_specular", _material->getSpecular()));
	}
}

void GeometryObject::updateMaterialState()
{
	// TODO: make sure pass (camera) is the correct node for forward shading
	osg::ref_ptr<osg::StateSet> ss = _materialNode->getOrCreateStateSet();

	if (_material->getUseTexture())
	{
		// to be implemented
	}
	else
	{
		// TODO: change to UBO later // and translucent
		ss->getUniform("u_albedo")->set(osg::Vec3f(_material->getAlbedo()));
		ss->getUniform("u_roughness")->set(_material->getRoughness());
		ss->getUniform("u_metallic")->set(_material->getMetallic());
		ss->getUniform("u_specular")->set(_material->getSpecular());
	}

}

osg::ref_ptr<osg::Program> GeometryObject::getPlainShader()
{
	if (_plainShader == NULL)
	{
		_plainShader = new osg::Program();
		_plainShader->addShader(osgDB::readShaderFile("Shaders/gbuffer_plain.vert"));
		_plainShader->addShader(osgDB::readShaderFile("Shaders/gbuffer_plain.frag"));
	}

	return _plainShader;
}

osg::ref_ptr<osg::Program> GeometryObject::getTexturedShader()
{
	if (_texturedShader == NULL)
	{
		_texturedShader = new osg::Program();
		_texturedShader->addShader(osgDB::readShaderFile("Shaders/gbuffer_textured.vert"));
		_texturedShader->addShader(osgDB::readShaderFile("Shaders/gbuffer_textured.frag"));
	}

	return _texturedShader;
}

osg::ref_ptr<osg::Program> GeometryObject::_plainShader;
osg::ref_ptr<osg::Program> GeometryObject::_texturedShader;