#include "stdafx.h" 
#include <ConfigSettings.h>

#include "TwGUIManager.h"
#include <unordered_map>

#include "Core.h"
#include "GeometryObject.h"
#include "Material.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "GeometryObjectManipulator.h"


TwGUIManager::TwGUIManager()
// Note, this flag assumes that you do not touch viewer manipulator settings
	: _cameraManipulatorActive(true), _manipulatorBits(0x0)
{
	_gm = Core::getWorldRef().getGeometryManager();
	_lm = Core::getWorldRef().getLightManager();
	_mm = Core::getWorldRef().getMaterialManager();

	TwInit(TW_OPENGL, NULL);
}


void TwGUIManager::initializeTwGUI()
{
	initMainBar();
	initManipuatorSelectorBar();
}

void TwGUIManager::initMainBar()
{
	g_twBar = TwNewBar("Main");
	TwDefine(" Main label='Kaboom Game Editor' size='300 600' color='96 216 224' position='16 110' valueswidth=140");

	TwAddButton(g_twBar, "Run Game",
		[](void *clientData) {
		Core::enableGameMode();
	}, NULL, " label='--> Run Game :)' ");

	TwAddSeparator(g_twBar, NULL, NULL);

	// Add option to disable/enable camera manipulator
	// I did not find a way to intercept the hover event to make
	// this automatic
	TwAddVarCB(g_twBar, "Cam Control", TW_TYPE_BOOL8,
		[](const void *value, void *clientData) {
		bool active = *static_cast<const bool *>(value);
		*(bool *)(clientData) = active;

		// FIXME: actually this is quite hacky, 
		// since only use a _cameraManipulatorActive as a 
		// trigger for this shared function
		if (active)
		{
			Core::enableCameraManipulator();
		}
		else
		{
			Core::disableCameraManipulator();
		}
	},
		[](void *value, void *clientData) {
		*(bool *)value = *(bool *)clientData;
	},
		&this->_cameraManipulatorActive, NULL);

	// 'Export to XML' button
	TwAddButton(g_twBar, "Export to XML",
		[](void *clientData) {
		exportXML();
	},
		NULL, NULL);

	TwAddSeparator(g_twBar, NULL, NULL);

	// Process geometries
	std::string GeomGroupName = "Edit GeometryObject";
	int index = 0;
	std::string posXLabel = "posX";
	std::string posYLabel = "posY";
	std::string posZLabel = "posZ";
	const std::unordered_map<std::string, GeometryObject *> &gmMap = _gm->getGeometryObjectMapRef();
	for (std::unordered_map<std::string, GeometryObject *>::const_iterator it = gmMap.begin();
		it != gmMap.end(); ++it)
	{
		const std::string &name = it->first;
		GeometryObject *geom = it->second;

		std::string nameGroupDef = " group='" + name + "' ";

		std::string indexStr = std::to_string(index);
		std::string posXVarName = posXLabel + indexStr;
		std::string posYVarName = posYLabel + indexStr;
		std::string posZVarName = posZLabel + indexStr;

		std::string posXDef = nameGroupDef + " label='" + posXLabel + "'";
		TwAddVarCB(g_twBar, posXVarName.c_str(), TW_TYPE_FLOAT,
			[](const void *value, void *clientData) {
			GeometryObject *obj = static_cast<GeometryObject *>(clientData);
			float posX = *(const float *)value;
			osg::Vec3 oldPos = obj->getTranslate();
			osg::Vec3 newPos = osg::Vec3(posX, oldPos.y(), oldPos.z());
			obj->setTranslate(newPos);
		},
			[](void *value, void *clientData) {
			GeometryObject *obj = static_cast<GeometryObject *>(clientData);
			float *posX = static_cast<float *>(value);

			osg::Vec3 pos = obj->getTranslate();
			*posX = pos.x();
		},
			geom, posXDef.c_str());

		std::string posYDef = nameGroupDef + " label='" + posYLabel + "'";
		TwAddVarCB(g_twBar, posYVarName.c_str(), TW_TYPE_FLOAT,
			[](const void *value, void *clientData) {
			GeometryObject *obj = static_cast<GeometryObject *>(clientData);
			float posY = *(const float *)value;
			osg::Vec3 oldPos = obj->getTranslate();
			osg::Vec3 newPos = osg::Vec3(oldPos.x(), posY, oldPos.z());
			obj->setTranslate(newPos);
		},
			[](void *value, void *clientData) {
			GeometryObject *obj = static_cast<GeometryObject *>(clientData);
			float *posY = static_cast<float *>(value);

			osg::Vec3 pos = obj->getTranslate();
			*posY = pos.y();
		},
			geom, posYDef.c_str());

		std::string posZDef = nameGroupDef + " label='" + posZLabel + "'";
		TwAddVarCB(g_twBar, posZVarName.c_str(), TW_TYPE_FLOAT,
			[](const void *value, void *clientData) {
			GeometryObject *obj = static_cast<GeometryObject *>(clientData);
			float posZ = *(const float *)value;
			osg::Vec3 oldPos = obj->getTranslate();
			osg::Vec3 newPos = osg::Vec3(oldPos.x(), oldPos.y(), posZ);
			obj->setTranslate(newPos);
		},
			[](void *value, void *clientData) {
			GeometryObject *obj = static_cast<GeometryObject *>(clientData);
			float *posZ = static_cast<float *>(value);

			osg::Vec3 pos = obj->getTranslate();
			*posZ = pos.z();
		},
			geom, posZDef.c_str());

		std::string rotationVarName = "rotation" + std::to_string(index);
		std::string rotationDef = nameGroupDef + " label='rotation'";
		TwAddVarCB(g_twBar, rotationVarName.c_str(), TW_TYPE_QUAT4F,
			[](const void *value, void *clientData) {
			GeometryObject *obj = static_cast<GeometryObject *>(clientData);
			const float *rotArr = static_cast<const float *>(value);

			// seems like ant tweak bar is nice enough to provide temp buffer for load/write
			// so that the following is safe
			osg::Quat newRot = osg::Quat(rotArr[0], rotArr[1], rotArr[2], rotArr[3]);
			obj->setRotation(newRot);
		},
			[](void *value, void *clientData) {
			GeometryObject *obj = static_cast<GeometryObject *>(clientData);
			float *rotArr = static_cast<float *>(value);

			osg::Quat rot = obj->getRotation();
			rotArr[0] = rot.x();
			rotArr[1] = rot.y();
			rotArr[2] = rot.z();
			rotArr[3] = rot.w();
		},
			geom, rotationDef.c_str());

		std::string moveStr = " Main/" + name + " group='" + GeomGroupName + "'";
		TwDefine(moveStr.c_str());

		index++;
	}

	//index = 0;

	// process lights
	std::string LightGroupStr = "Edit Light";
	for (int i = 0; i < _lm->getNumLights(); i++)
	{
		Light *l = _lm->getLight(i);
		std::string name = l->getName();
		std::string nameGroupDef = " group='" + name + "' ";

		std::string indexStr = std::to_string(index);
		std::string posXVarName = posXLabel + indexStr;
		std::string posYVarName = posYLabel + indexStr;
		std::string posZVarName = posZLabel + indexStr;

		if (l->getLightType() == DIRECTIONAL)
		{
			DirectionalLight *dl = l->asDirectionalLight();

			// lightDir ( to light, not from light ) 
			std::string dirToWorldVarName = "dirToWorld" + std::to_string(index);
			std::string dirToWorldDef = nameGroupDef + " label='dirToWorld'";
			TwAddVarCB(g_twBar, dirToWorldVarName.c_str(), TW_TYPE_DIR3F,
				[](const void *value, void *clientData) {
				DirectionalLight *dl = static_cast<DirectionalLight *>(clientData);
				const float *arr = static_cast<const float *>(value);
				osg::Vec3 dir = osg::Vec3(arr[0], arr[1], arr[2]);
				dl->setLightToWorldDirection(dir);
			},
				[](void *value, void *clientData) {
				DirectionalLight *dl = static_cast<DirectionalLight *>(clientData);
				const osg::Vec3 &dir = dl->getLightToWorldDirection();
				float *arr = static_cast<float *>(value);
				arr[0] = dir.x(); arr[1] = dir.y(); arr[2] = dir.z();
			},
				dl, dirToWorldDef.c_str());

			// TODO: color, later.... Too tired
			// TW_TYPE_COLOR3F
			std::string dirLightColorVarName = "Color" + std::to_string(index);
			std::string dirLightColorDef = nameGroupDef + " label='Color'";
			TwAddVarCB(g_twBar, dirLightColorVarName.c_str(), TW_TYPE_COLOR3F,
				[](const void *value, void *clientData) {
				DirectionalLight *dl = static_cast<DirectionalLight *>(clientData);
				const float *arr = static_cast<const float *>(value);
				osg::Vec3 color = osg::Vec3(arr[0], arr[1], arr[2]);
				dl->setColor(color);
			},
				[](void *value, void *clientData) {
				DirectionalLight *dl = static_cast<DirectionalLight *>(clientData);
				const osg::Vec3 &color = dl->getColor();
				float *arr = static_cast<float *>(value);
				arr[0] = color.x(); arr[1] = color.y(); arr[2] = color.z();
			}, dl, dirLightColorDef.c_str());
		}
		else if (l->getLightType() == POINTLIGHT)
		{
			PointLight *pl = l->asPointLight();

			std::string posXDef = nameGroupDef + " label='" + posXLabel + "'";
			TwAddVarCB(g_twBar, posXVarName.c_str(), TW_TYPE_FLOAT,
				[](const void *value, void *clientData) {
				float posX = *(const float *)value;
				PointLight *pl = static_cast<PointLight *>(clientData);
				const osg::Vec3 &oldPos = pl->getPosition();
				pl->setPosition(osg::Vec3(posX, oldPos.y(), oldPos.z()));
			},
				[](void *value, void *clientData) {
				float *posX = static_cast<float *>(value);
				PointLight *pl = static_cast<PointLight *>(clientData);
				const osg::Vec3 &pos = pl->getPosition();
				*posX = pos.x();

			}, pl, posXDef.c_str());

			std::string posYDef = nameGroupDef + " label='" + posYLabel + "'";
			TwAddVarCB(g_twBar, posYVarName.c_str(), TW_TYPE_FLOAT,
				[](const void *value, void *clientData) {
				float posY = *(const float *)value;
				PointLight *pl = static_cast<PointLight *>(clientData);
				const osg::Vec3 &oldPos = pl->getPosition();

				pl->setPosition(osg::Vec3(oldPos.x(), posY, oldPos.z()));
			},
				[](void *value, void *clientData) {
				float *posY = static_cast<float *>(value);
				PointLight *pl = static_cast<PointLight *>(clientData);
				const osg::Vec3 &pos = pl->getPosition();
				*posY = pos.y();

			}, pl, posYDef.c_str());

			std::string posZDef = nameGroupDef + " label='" + posZLabel + "'";
			TwAddVarCB(g_twBar, posZVarName.c_str(), TW_TYPE_FLOAT,
				[](const void *value, void *clientData) {
				float posZ = *(const float *)value;
				PointLight *pl = static_cast<PointLight *>(clientData);
				const osg::Vec3 &oldPos = pl->getPosition();
				pl->setPosition(osg::Vec3(oldPos.x(), oldPos.y(), posZ));
			},
				[](void *value, void *clientData) {
				float *posZ = static_cast<float *>(value);
				PointLight *pl = static_cast<PointLight *>(clientData);
				const osg::Vec3 &pos = pl->getPosition();
				*posZ = pos.z();

			}, pl, posZDef.c_str());

			// radius
			std::string radiusDefStr = nameGroupDef + " label='radius' min=0";
			std::string radiusVarStr = "radius" + indexStr;
			TwAddVarCB(g_twBar, radiusVarStr.c_str(), TW_TYPE_FLOAT,
				[](const void *value, void *clientData) {
				float rad = *static_cast<const float *>(value);
				PointLight *pl = static_cast<PointLight *>(clientData);
				pl->setRadius(rad);
			},
				[](void *value, void *clientData)
			{
				float *val = static_cast<float *>(value);
				PointLight *pl = static_cast<PointLight *>(clientData);
				*val = pl->getRadius();
			},
				pl, radiusDefStr.c_str());

			// TODO: add color ..
			// TODO: color, later.... Too tired
			// TW_TYPE_COLOR3F
			std::string ptLightColorVarName = "Color" + std::to_string(index);
			std::string ptLightColorDef = nameGroupDef + " label='Color'";
			TwAddVarCB(g_twBar, ptLightColorVarName.c_str(), TW_TYPE_COLOR3F,
				[](const void *value, void *clientData) {
				PointLight *pl = static_cast<PointLight *>(clientData);
				const float *arr = static_cast<const float *>(value);
				osg::Vec3 color = osg::Vec3(arr[0], arr[1], arr[2]);
				pl->setColor(color);
			},
				[](void *value, void *clientData) {
				PointLight *pl = static_cast<PointLight *>(clientData);
				const osg::Vec3 &color = pl->getColor();
				float *arr = static_cast<float *>(value);
				arr[0] = color.x(); arr[1] = color.y(); arr[2] = color.z();
			}, pl, ptLightColorDef.c_str());
		}

		std::string moveStr = " Main/" + name + " group='" + LightGroupStr + "'";
		TwDefine(moveStr.c_str());

		index++;
	}
}

void TwGUIManager::initManipuatorSelectorBar()
{
	g_manipulatorSelectorBar = TwNewBar("Manipulators");
	TwDefine(" Manipulators label='Manipulators' size='300 90' color='96 96 96' position='16 16'");
	TwAddVarCB(g_manipulatorSelectorBar, "TabboxM", TW_TYPE_BOOL8, 
		[](const void *value, void *clientData)	{
		bool tf = *(bool *)value;
		bool res = GeometryObjectManipulator::setVisible(tf);
		if (!res) return;
		int &bits = *(int *)clientData;
		bits &= 0x1;
		tf ? bits |= 0x1 : bits ^= 0x1;
		tf ? GeometryObjectManipulator::changeCurrentManipulatorType(TabBoxDragger) : "";
		},
		[](void *value, void *clientData)
		{
			int bits = *(int *)clientData;
			bool &v = *(bool *)value;
			(bits & 0x1) != 0 ? v = true : v = false;
		},
		&_manipulatorBits, " label=TabBox");

		TwAddVarCB(g_manipulatorSelectorBar, "RotateM", TW_TYPE_BOOL8,
			[](const void *value, void *clientData)	{
			bool tf = *(bool *)value;
			bool res = GeometryObjectManipulator::setVisible(tf);
			if (!res) return;
			int &bits = *(int *)clientData;
			bits &= 0x2;
			tf ? bits |= 0x2 : bits ^= 0x2;
			tf ? GeometryObjectManipulator::changeCurrentManipulatorType(TrackballDragger) : "";
		},
			[](void *value, void *clientData)
		{
			int bits = *(int *)clientData;
			bool &v = *(bool *)value;
			(bits & 0x2) != 0 ? v = true : v = false;
		},
			&_manipulatorBits, " label=Rotate");

		TwAddVarCB(g_manipulatorSelectorBar, "AxisM", TW_TYPE_BOOL8,
			[](const void *value, void *clientData)	{
			bool tf = *(bool *)value;
			bool res = GeometryObjectManipulator::setVisible(tf);
			if (!res) return;
			int &bits = *(int *)clientData;
			bits &= 0x4;
			tf ? bits |= 0x4 : bits ^= 0x4;
			tf ? GeometryObjectManipulator::changeCurrentManipulatorType(TranslateAxisDragger) : "";
		},
			[](void *value, void *clientData)
		{
			int bits = *(int *)clientData;
			bool &v = *(bool *)value;
			(bits & 0x4) != 0 ? v = true : v = false;
		},
			&_manipulatorBits, " label=Axis");

}

void TW_CALL TwGUIManager::loadModelFunc(void* clientData)
{
	TwGUIManager* manager = (TwGUIManager*)clientData;
	/*if (manager && manager->_scene.valid())
	{
	manager->_scene->removeChild(0, manager->_scene->getNumChildren());
	manager->_scene->addChild(osgDB::readNodeFile(g_fileName));
	}*/

	// TODO: implement
}

// TODO: bug, scrolling frequently will crash the program
// TODO: anttweakbar by default uses opengl convention for world coordinates on widgets(arrow)
// Need to change them to conform to osg convention
void TwGUIManager::updateEvents() const
{
	// update manipulatorBits from GeometryManipulator
	if (GeometryObjectManipulator::isVisible())
	{
		switch (GeometryObjectManipulator::getCurrentManipulatorType())
		{
		case TabBoxDragger:
			*(int *)&_manipulatorBits = 0x1;
			break;
		case TrackballDragger:
			*(int *)&_manipulatorBits = 0x2;
			break;
		case TranslateAxisDragger:
			*(int *)&_manipulatorBits = 0x4;
			break;
		}
	}

	unsigned int size = _eventsToHandle.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		if (_eventsToHandle.front() == NULL) return; // attemps to fix crashing; but reason of NULL events unknown
		const osgGA::GUIEventAdapter& ea = *(_eventsToHandle.front());
		float x = ea.getX(), y = ea.getWindowHeight() - ea.getY();
		switch (ea.getEventType())
		{
		case osgGA::GUIEventAdapter::PUSH:
			TwMouseMotion(x, y);
			TwMouseButton(TW_MOUSE_PRESSED, getTwButton(ea.getButton()));
			break;
		case osgGA::GUIEventAdapter::RELEASE:
			TwMouseMotion(x, y);
			TwMouseButton(TW_MOUSE_RELEASED, getTwButton(ea.getButton()));
			break;
		case osgGA::GUIEventAdapter::DRAG:
		case osgGA::GUIEventAdapter::MOVE:
			TwMouseMotion(x, y);
			break;
		case osgGA::GUIEventAdapter::KEYDOWN:
		{
			bool useCtrl = false;
			if (ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL)
			{
				useCtrl = true;
			}
			else if (ea.getKey() == 'g')
			{
				GeometryObjectManipulator::changeCurrentManipulatorType(TranslateAxisDragger);
			}
			else if (ea.getKey() == 'r')
			{
				GeometryObjectManipulator::changeCurrentManipulatorType(TrackballDragger);
			}
			else if (ea.getKey() == 's')
			{
				GeometryObjectManipulator::changeCurrentManipulatorType(TabBoxDragger);

			}
			TwKeyPressed(getTwKey(ea.getKey(), useCtrl), getTwModKeyMask(ea.getModKeyMask()));
		}
			break;
		default: break;
		}
		const_cast<TwGUIManager*>(this)->_eventsToHandle.pop();
	}
}

bool TwGUIManager::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	// TODO: have some bugs, some times insert some null events, making program crash 
	switch (ea.getEventType())
	{
	case osgGA::GUIEventAdapter::FRAME:  // Update transform values
		/*if (_scene.valid())
		{
		osg::Vec3 pos(position[0], position[1], position[2]);
		osg::Quat quat(rotation[0], rotation[1], rotation[2], rotation[3]);
		_scene->setMatrix(osg::Matrix::rotate(quat) * osg::Matrix::translate(pos));
		}*/
		return false;
	}

	// As AntTweakBar handle all events within the OpenGL context, we have to record the event here
	// and process it later in the draw callback
	_eventsToHandle.push(&ea);
	return false;
}

void TwGUIManager::operator()(osg::RenderInfo& renderInfo) const
{
	osg::Viewport* viewport = renderInfo.getCurrentCamera()->getViewport();
	if (viewport) TwWindowSize(viewport->width(), viewport->height());
	updateEvents();
	TwDraw();
}

TwMouseButtonID TwGUIManager::getTwButton(int button) const
{
	switch (button)
	{
	case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON: return TW_MOUSE_LEFT;
	case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON: return TW_MOUSE_MIDDLE;
	case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON: return TW_MOUSE_RIGHT;
	}
	return static_cast<TwMouseButtonID>(0);
}

int TwGUIManager::getTwKey(int key, bool useCtrl) const
{
	switch (key)
	{
	case osgGA::GUIEventAdapter::KEY_BackSpace: return TW_KEY_BACKSPACE;
	case osgGA::GUIEventAdapter::KEY_Tab: return TW_KEY_TAB;
	case osgGA::GUIEventAdapter::KEY_Return: return TW_KEY_RETURN;
	case osgGA::GUIEventAdapter::KEY_Escape: return TW_KEY_ESCAPE;
	case osgGA::GUIEventAdapter::KEY_Left: return TW_KEY_LEFT;
	case osgGA::GUIEventAdapter::KEY_Right: return TW_KEY_RIGHT;
	case osgGA::GUIEventAdapter::KEY_Up: return TW_KEY_UP;
	case osgGA::GUIEventAdapter::KEY_Down: return TW_KEY_DOWN;
	case osgGA::GUIEventAdapter::KEY_Home: return TW_KEY_HOME;
	case osgGA::GUIEventAdapter::KEY_End: return TW_KEY_END;
	case osgGA::GUIEventAdapter::KEY_Insert: return TW_KEY_INSERT;
	case osgGA::GUIEventAdapter::KEY_Delete: return TW_KEY_DELETE;
	case osgGA::GUIEventAdapter::KEY_Page_Up: return TW_KEY_PAGE_UP;
	case osgGA::GUIEventAdapter::KEY_Page_Down: return TW_KEY_PAGE_DOWN;
	case osgGA::GUIEventAdapter::KEY_F1: return TW_KEY_F1;
	case osgGA::GUIEventAdapter::KEY_F2: return TW_KEY_F2;
	case osgGA::GUIEventAdapter::KEY_F3: return TW_KEY_F3;
	case osgGA::GUIEventAdapter::KEY_F4: return TW_KEY_F4;
	case osgGA::GUIEventAdapter::KEY_F5: return TW_KEY_F5;
	case osgGA::GUIEventAdapter::KEY_F6: return TW_KEY_F6;
	case osgGA::GUIEventAdapter::KEY_F7: return TW_KEY_F7;
	case osgGA::GUIEventAdapter::KEY_F8: return TW_KEY_F8;
	case osgGA::GUIEventAdapter::KEY_F9: return TW_KEY_F9;
	case osgGA::GUIEventAdapter::KEY_F10: return TW_KEY_F10;
	case osgGA::GUIEventAdapter::KEY_F11: return TW_KEY_F11;
	case osgGA::GUIEventAdapter::KEY_F12: return TW_KEY_F12;
	}
	if (useCtrl && key < 27) key += 'a' - 1;
	return key;
}

int TwGUIManager::getTwModKeyMask(int modkey) const
{
	int twModkey = 0;
	if (modkey&osgGA::GUIEventAdapter::MODKEY_SHIFT) twModkey |= TW_KMOD_SHIFT;
	if (modkey&osgGA::GUIEventAdapter::MODKEY_ALT) twModkey |= TW_KMOD_ALT;
	if (modkey&osgGA::GUIEventAdapter::MODKEY_CTRL) twModkey |= TW_KMOD_CTRL;
	return twModkey;
}


void TwGUIManager::write(std::ofstream &f, int tabs, std::string s)
{
	for (int i = 0; i < tabs; i++) {
		f << "\t";
	}

	f << s << "\n";
}

std::string TwGUIManager::addTags(std::string tag, std::string s)
{
	return "<" + tag + ">" + s + "</" + tag + ">";
}

std::string TwGUIManager::tagify(std::string tag, std::string s)
{
	return addTags(tag, s);
}

std::string TwGUIManager::tagify(std::string tag, float f)
{
	return addTags(tag, std::to_string(f));
}

std::string TwGUIManager::tagify(std::string tag, bool b)
{
	int i = b ? 1 : 0;
	return addTags(tag, std::to_string(i));
}

std::string TwGUIManager::tagify(std::string tag, osg::Vec3 &v)
{
	std::string ret = "";

	for (int i = 0; i < v.num_components; i++) {
		ret = ret + std::to_string(v[i]);

		if (i < v.num_components - 1) {
			ret = ret + " ";
		}
	}

	return addTags(tag, ret);
}

std::string TwGUIManager::tagify(std::string tag, osg::Vec4 &v)
{
	std::string ret = "";

	for (int i = 0; i < v.num_components; i++) {
		ret = ret + std::to_string(v[i]);

		if (i < v.num_components - 1) {
			ret = ret + " ";
		}
	}

	return addTags(tag, ret);
}

void TwGUIManager::exportXML()
{
	// Might wanna move this code somewhere else
	ConfigSettings* config = ConfigSettings::config;
	std::string str_export_xml = "";
	config->getValue(ConfigSettings::str_export_xml, str_export_xml);

	// Open file to write
	int tabs = 0;
	std::ofstream f;
	f.open(str_export_xml);

	// Headers
	write(f, tabs, "<?xml version=\"1.0\" encoding=\"utf - 8\"?>");
	write(f, tabs, "<world>");
	tabs++;

	MaterialManager* mm = Core::getWorldRef().getMaterialManager();
	GeometryObjectManager* gm = Core::getWorldRef().getGeometryManager();
	LightManager* lm = Core::getWorldRef().getLightManager();

	// FOR MATERIALS
	const std::unordered_map<std::string, Material *> &mmMatMap = mm->getMaterialMapRef();
	for (std::unordered_map<std::string, Material *>::const_iterator it = mmMatMap.begin();
		it != mmMatMap.end(); ++it)
	{
		const std::string &name = it->first;
		Material *mat = it->second;			// Get Material object

		std::string type = (mat->getUseTexture()) ? "textured" : "plain";
		std::string matHeader = "<material name=\"" + name + "\" type=\"" + type + "\">";

		write(f, tabs, matHeader);
		tabs++;

		// For plain materials
		if (type == "plain") {
			osg::Vec3 alb = mat->getAlbedo();
			float rough = mat->getRoughness();
			float specular = mat->getSpecular();
			float metallic = mat->getMetallic();

			write(f, tabs, tagify("albedo", alb));
			write(f, tabs, tagify("roughness", rough));
			write(f, tabs, tagify("specular", specular));
			write(f, tabs, tagify("metallic", metallic));
		}
		// For textured materials
		else if (type == "textured") {
			std::string albPath = mat->getAlbedoTexturePath();
			std::string roughPath = mat->getRoughnessTexturePath();
			std::string metallicPath = mat->getMetallicTexturePath();
			std::string normalPath = mat->getNormalMapTexturePath();

			write(f, tabs, tagify("albedoTex", albPath));
			write(f, tabs, tagify("roughnessTex", roughPath));
			write(f, tabs, tagify("metallicTex", metallicPath));
			write(f, tabs, tagify("normalPath", normalPath));
		}

		tabs--;
		write(f, tabs, "</material>\n");
	}

	// FOR GEOMETRY OBJECTS
	const std::unordered_map<std::string, GeometryObject *> &gmMap = gm->getGeometryObjectMapRef();
	for (std::unordered_map<std::string, GeometryObject *>::const_iterator it = gmMap.begin();
		it != gmMap.end(); ++it)
	{
		const std::string &name = it->first;
		GeometryObject *geom = it->second;			// Get GeometryObject

		std::string matHeader = "<model name=\"" + name + "\">";

		write(f, tabs, matHeader);
		tabs++;

		// Geometry properties
		std::string file = geom->getFileName();
		std::string mat = geom->getMaterial()->getName();
		osg::Vec3 pos = geom->getTranslate();
		osg::Vec4 rot = geom->getRotation().asVec4();
		// TODO: collider

		std::string matTag = "<material name=\"" + mat + "\" />";

		write(f, tabs, tagify("file", file));
		write(f, tabs, matTag);
		write(f, tabs, tagify("position", pos));
		write(f, tabs, tagify("orientation", rot));
		//write(f, tabs, tagify("collider", ));

		tabs--;
		write(f, tabs, "</model>\n");
	}

	// FOR LIGHT OBJECTS
	const std::unordered_map<std::string, Light *> &lmMap = lm->getLightMapRef();
	for (std::unordered_map<std::string, Light *>::const_iterator it = lmMap.begin();
		it != lmMap.end(); ++it)
	{
		const std::string &name = it->first;
		Light *light = it->second;			// Get Light object

		std::string type = "";

		switch (light->getLightType()) {
		case POINTLIGHT:
			type = "point";
			break;
		case DIRECTIONAL:
			type = "directional";
			break;
		}

		std::string ltHeader = "<light name=\"" + name + "\" type=\"" + type + "\">";

		write(f, tabs, ltHeader);
		tabs++;

		osg::Vec3 color = light->getColor();
		bool shadow = light->getCastShadow();

		write(f, tabs, tagify("color", color));
		write(f, tabs, tagify("castShadow", shadow));

		// Light properties
		if (type == "point") {
			osg::Vec3 pos = light->getPosition();
			float radius = ((PointLight *)light)->getRadius();

			write(f, tabs, tagify("position", pos));
			write(f, tabs, tagify("radius", radius));
		}
		// For textured materials
		else if (type == "directional") {
			osg::Vec3 dir = ((DirectionalLight *)light)->getLightToWorldDirection();

			write(f, tabs, tagify("direction", dir));
		}

		tabs--;
		write(f, tabs, "</light>\n");
	}

	// Footers
	tabs--;
	write(f, tabs, "</world>");
	f.close();			// Close file
}
