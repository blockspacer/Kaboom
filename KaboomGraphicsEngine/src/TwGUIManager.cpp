#include <Windows.h>	// For "Open File" dialog
#include "stdafx.h" 

#include <util/ConfigSettings.h>

#include "TwGUIManager.h"
#include <unordered_map>

#include "Core.h"
#include "GeometryObject.h"
#include "GeometryCache.h"
#include "Material.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "GeometryObjectManipulator.h"
#include <osg/ComputeBoundsVisitor>

//This is needed for virtually 
//everything in BrowseFolder.
#include <shlobj.h>

const float PI_F = 3.14159265358979f;

int TwGUIManager::_index = 0;
std::vector<UndoRedoNode*> TwGUIManager::_undos = std::vector<UndoRedoNode*>();
std::vector<UndoRedoNode*> TwGUIManager::_redos = std::vector<UndoRedoNode*>();
UndoRedoNode* TwGUIManager::_currChange = NULL;

TwGUIManager::TwGUIManager()
// Note, this flag assumes that you do not touch viewer manipulator settings
	: _cameraManipulatorActive(true), _manipulatorBits(0x0)
{
	_gm = Core::getWorldRef().getGeometryManager();
	_lm = Core::getWorldRef().getLightManager();
	_mm = Core::getWorldRef().getMaterialManager();
	_freezeGUI = false;
	_allBarMinimized = false;
	_scrollPos = 0;

	nameToCopy = "";

	TwInit(TW_OPENGL, NULL);
}

void TwGUIManager::minimizeAllBars()
{
	TwDefine(" Main iconified=true ");
	TwDefine(" Lights iconified=true ");
	TwDefine(" Manipulators iconified=true ");
	TwDefine(" Add iconified=true ");
	TwDefine(" Plain_Materials iconified=true ");
	TwDefine(" Textured_Materials iconified=true ");
	_allBarMinimized = true;
}

void TwGUIManager::maximizeAllBars()
{
	TwDefine(" Main iconified=false ");
	TwDefine(" Lights iconified=false ");
	TwDefine(" Manipulators iconified=false ");
	TwDefine(" Add iconified=false ");
	TwDefine(" Plain_Materials iconified=false ");
	TwDefine(" Textured_Materials iconified=false ");
	_allBarMinimized = false;
}

void TwGUIManager::initializeTwGUI()
{
	_index = 0;

	initMainBar();
	initLightBar();
	initManipuatorSelectorBar();
	initPlainMaterialBar();
	initTexturedMaterialBar();
	initAddBar();				// should be done last
}

void TwGUIManager::initMainBar()
{
	g_twBar = TwNewBar("Main");
	TwDefine(" Main label='Kaboom Game Editor' size='300 400' color='96 216 224' position='16 110' valueswidth=140");

	TwAddButton(g_twBar, "Run Game",
		[](void *clientData) {
		Core::enableStartScreen();
	}, NULL, " label='--> Run Game :)' ");


	// 'Load World File' button
	TwAddButton(g_twBar, "Load World File",
		[](void *clientData) {
		loadWorldXML();
	},
		NULL, NULL);

	// 'Export to XML' button
	TwAddButton(g_twBar, "Export to XML",
		[](void *clientData) {
		exportXML();
	},
		NULL, NULL);

	TwAddSeparator(g_twBar, NULL, NULL);
/*
	TwAddVarCB(g_twBar, "Show LibRocketGUI", TW_TYPE_BOOL8,
		[](const void *data, void *clientData) {
		bool tick = *(bool *)data;
		if (tick)
		{
			Core::enableLibRocketInEditorGUI();
		}
		else
		{
			Core::disableLibRocketInEditorGUI();
		}
	}, [](void *data, void *clientData) {
		*(bool *)data = Core::isLibRocketInEditorGUIEnabled();
	}, NULL, NULL)*/;

	std::string camGroupDef = " group='Cam Settings' ";

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
		&this->_cameraManipulatorActive, camGroupDef.c_str());

	TwAddVarCB(g_twBar, "FPS Cam", TW_TYPE_BOOL8, 
		[](const void *data, void *cliantData) {
		bool isFps = *static_cast<const bool *>(data);
		if (isFps) Core::switchToFirstPersonCamManipulator();
		else Core::switchToTrackBallCamManipulator();
		}, 
		[](void *data, void *clientData) {
			if (Core::getCurrCamManipulatorType() == Core::TRACKBALL)
			{
				*(bool *)data = false;
			}
			else
			{
				*(bool *)data = true;
			}
		}, NULL, camGroupDef.c_str());

	TwAddVarCB(g_twBar, "FPS Cam Speed", TW_TYPE_FLOAT, 
		[](const void *data, void *clientData) {
		Core::setEditorFPSCamWalkingSpeed(*(float *)data);
	}, [](void *data, void *clientData) {
		*(float *)data = Core::getEditorFPSCamWalkingSpeed();
	}, NULL, (camGroupDef + "step=1.0 min=0.0 max=100.0").c_str());

	TwAddVarCB(g_twBar, "Change Projection", TW_TYPE_BOOL8, 
		[](const void *data, void *clientData) {
		bool b = *static_cast<const bool *>(data);
		Core::setAllowChangeEditorProjection(b);
	}, [](void *data, void *clientData) {
		bool b = Core::allowChangeEditorProjection();
		*(bool *)data = b;
	}, NULL, camGroupDef.c_str());

	TwAddVarCB(g_twBar, "Camera Fovy", TW_TYPE_FLOAT, 
		[](const void *data, void *clientData) {
		const float *fov = static_cast<const float *>(data);
		Core::getMainCamera().setFovYAndUpdate(*fov);
	}, [](void *data, void *clientData) {
		float fov = Core::getMainCamera().getFovY();
		*(float *)data = fov;
	}, NULL, (camGroupDef + " min=20 max=140 ").c_str());

	TwAddVarCB(g_twBar, "Camera Near Plane", TW_TYPE_FLOAT,
		[](const void *data, void *clientData) {
		const float *cam_near = static_cast<const float *>(data);
		Core::getMainCamera().setNearAndUpdate(*cam_near);
	}, [](void *data, void *clientData) {
		float fov = Core::getMainCamera().getNearPlane();
		*(float *)data = fov;
	}, NULL, (camGroupDef + " min=0.1 max=10000").c_str());

	TwAddVarCB(g_twBar, "Camera Far Plane", TW_TYPE_FLOAT,
		[](const void *data, void *clientData) {
		const float *cam_far = static_cast<const float *>(data);
		Core::getMainCamera().setFarAndUpdate(*cam_far);
	}, [](void *data, void *clientData) {
		float fov = Core::getMainCamera().getFarPlane();
		*(float *)data = fov;
	}, NULL, (camGroupDef + " min=0.1 max=10000").c_str());

	std::string foldedStr = " Main/'Cam Settings' opened=false ";
	TwDefine(foldedStr.c_str());

	TwAddSeparator(g_twBar, NULL, NULL);

	initAddBarHelper();
	//index = 0;
}


void TwGUIManager::initAddBarHelper()
{
	// Process geometries
	const std::unordered_map<std::string, GeometryObject *> &gmMap = _gm->getGeometryObjectMapRef();
	for (std::unordered_map<std::string, GeometryObject *>::const_iterator it = gmMap.begin();
		it != gmMap.end(); ++it)
	{
		const std::string &name = it->first;
		GeometryObject *geom = it->second;

		// Moved code to a function
		addModelToGUI(g_twBar, geom, GEOM_GROUP_NAME, _index);
	}
}

void TwGUIManager::initLightBar()
{
	g_lightBar = TwNewBar("Lights");
	TwDefine(" Lights label='Lights' size='300 180' color='185 185 50' position='16 515'");

	initLightBarHelper();
}

void TwGUIManager::initLightBarHelper()
{
	// process lights
	for (int i = 0; i < _lm->getNumLights(); i++)
	{
		Light *l = _lm->getLight(i);

		// Moved code to a function
		addLightToGUI(g_lightBar, l, LIGHT_GROUP_NAME, _index);
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

void TwGUIManager::initAddBar()
{
	g_addBar = TwNewBar("Add");
	TwDefine(" Add size='250 120' color='216 96 224' position='1025 16'");

	// 'Add model' button
	TwAddButton(g_addBar, "Add model",
		[](void *clientData) {

		// For Windows functions
		const size_t newsize = 4096;
		wchar_t fromPath[newsize];
		wchar_t toPath[newsize];
		wchar_t newFile[newsize];

		std::string filePath = "";
		bool validFile = openFile(filePath);

		if (validFile) {
			std::string fileName = getFileName(filePath);		// Get the file name (without the path)
			strToWCchar(fromPath, filePath);

			// Get the destination path
			ConfigSettings* config = ConfigSettings::config;
			std::string str_mediaPath = "";
			config->getValue(ConfigSettings::str_mediaFilePath, str_mediaPath);

			std::string geomAssetPath = str_mediaPath + "Assets/GeometryObject/";

			//// Append the file name to the destination path -> new file location
			//strToWCchar(toPath, str_mediaPath);
			//strToWCchar(newFile, fileName);
			//wcscat_s(toPath, newFile);

			//CopyFile(fromPath, toPath, FALSE);
			//DWORD dw = GetLastError();							// [Debug] Should be 0

			// Add model to geometry manager
			OSG_WARN << "Loading geometry object... " << fileName;
			osg::Node *model = Core::getWorldRef().getGeometryCache()->getNodeByFileName(fileName);
			if (model != NULL)
			{
				OSG_WARN << "Successfully! " << std::endl;
			}
			else
			{
				OSG_WARN << "Failed ! May crash the program..." << std::endl;
				return;
			}

			GeometryObjectManager* gm = Core::getWorldRef().getGeometryManager();

			// Get the input name
			std::string modelName;
			std::cout << "Enter model name: ";
			std::cin >> modelName;

			std::string objAssetFullPath = geomAssetPath + "/";

			// Append the file name to the destination path -> new file location
			strToWCchar(toPath, objAssetFullPath);
			strToWCchar(newFile, fileName);

			CreateDirectory(toPath, NULL);

			wcscat_s(toPath, newFile);
			CopyFile(fromPath, toPath, FALSE);
			DWORD dw = GetLastError();							// [Debug] Should be 0

			if (gm->addGeometry(modelName, model, fileName)) {
				GeometryObject* geom = gm->getGeometryObject(modelName);

				// Calculate light position relative to center of screen
				Camera cam = Core::getMainCamera();
				osg::Vec3 position = cam.getScreenCenterCoord(0.3f);
				geom->setTranslate(position);

				addModelToGUI((TwBar*)clientData, geom, GEOM_GROUP_NAME, _index);
			}
		}
	},
		g_twBar, NULL);

	// 'Add point light' button
	TwAddButton(g_addBar, "Add point light",
		[](void *clientData) {

		// Light default properties
		osg::Vec3 color;
		osg::Vec3 position;
		float radius = 100.0f;		// Not sure if this is a good value
		bool doShadow = false;

		// Add light to light manager
		LightManager* lm = Core::getWorldRef().getLightManager();

		// Get the input name
		std::string lightName;
		std::cout << "Enter point light name: ";
		std::cin >> lightName;

		// Calculate light position relative to center of screen
		Camera cam = Core::getMainCamera();
		position = cam.getScreenCenterCoord(0.1f);

		if (lm->addPointLight(lightName, position, color, radius, doShadow)) {
			Light* lt = lm->getLight(lightName);

			addLightToGUI((TwBar*)clientData, lt, LIGHT_GROUP_NAME, _index);
		}
	},
		g_twBar, NULL);

	// 'Add plain material' button
	TwAddButton(g_addBar, "Add plain material",
		[](void *clientData) {

		// TODO: Find out which default values to use
		// Material default properties
		osg::Vec3 albedoColor = osg::Vec3(0.3, 0.3, 0.7);
		float roughness = 0.4f;
		float specular = 0.5f;
		float metallic = 0.2f;

		// Add material to material manager
		MaterialManager* mm = Core::getWorldRef().getMaterialManager();

		// Get the input name
		std::string matName;
		std::cout << "Enter material name: ";
		std::cin >> matName;

		if (mm->createPlainMaterial(matName, albedoColor, roughness, specular, metallic)) {
			Material* mat = mm->getMaterial(matName);

			addPlainMaterialToGUI((TwBar*)clientData, mat, PLAIN_MATERIAL_GROUP_NAME, _index);
		}
	},
		g_plainMaterialBar, NULL);

	// 'Add textured material' button
	TwAddButton(g_addBar, "Add textured material",
		[](void *clientData) {

		// Material default properties
		std::string albedoPath = "";
		std::string roughnessPath = "";
		std::string metallicPath = "";
		std::string normalMapPat = "";

		// Add material to material manager
		MaterialManager* mm = Core::getWorldRef().getMaterialManager();

		// Get the input name
		std::string matName;
		std::cout << "Enter material name: ";
		std::cin >> matName;

		if (mm->createTexturedMaterial(matName, albedoPath, roughnessPath, metallicPath, normalMapPat)) {
			Material* mat = mm->getMaterial(matName);

			addTexturedMaterialToGUI((TwBar*)clientData, mat, TEXTURED_MATERIAL_GROUP_NAME, _index);
		}
	},
		g_texturedMaterialBar, NULL);

	// 'Change cubemap' button
	TwAddButton(g_addBar, "Change cubemap",
		[](void *clientData) {

		// For Windows functions
		const size_t newsize = 4096;
		wchar_t fromPath[newsize];
		wchar_t toPath[newsize];
		wchar_t newFile[newsize];

		std::string filePath = "";
		bool validFile = openFile(filePath);

		if (validFile) {
			std::string fileName = getFileName(filePath);		// Get the file name (without the path)
			strToWCchar(fromPath, filePath);

			// Get the destination path
			ConfigSettings* config = ConfigSettings::config;
			std::string str_mediaPath = "";
			config->getValue(ConfigSettings::str_mediaFilePath, str_mediaPath);

			str_mediaPath += "Assets/EnvironmentMap/";

			// Append the file name to the destination path -> new file location
			strToWCchar(toPath, str_mediaPath);
			strToWCchar(newFile, fileName);
			wcscat_s(toPath, newFile);

			CopyFile(fromPath, toPath, FALSE);
			DWORD dw = GetLastError();							// [Debug] Should be 0

			Core::setEnvironmentMapVerticalCross(fileName);
			Core::requestPrefilterCubeMap();
		}
	},
		NULL, NULL);
}

void TwGUIManager::initPlainMaterialBar()
{
	g_plainMaterialBar = TwNewBar("Plain_Materials");
	TwDefine(" Plain_Materials label='Plain Materials' size='250 240' color='96 216 96' position='1025 140' valueswidth=100");

	initPlainMaterialBarHelper();
}

void TwGUIManager::initPlainMaterialBarHelper()
{
	// Process materials
	const std::unordered_map<std::string, Material *> &mmMap = _mm->getMaterialMapRef();
	for (std::unordered_map<std::string, Material *>::const_iterator it = mmMap.begin();
		it != mmMap.end(); ++it)
	{
		const std::string &name = it->first;
		Material *mat = it->second;

		if (mat->getUseTexture()) continue;

		// Moved code to a function
		addPlainMaterialToGUI(g_plainMaterialBar, mat, PLAIN_MATERIAL_GROUP_NAME, _index);
	}
}

void TwGUIManager::initTexturedMaterialBar()
{
	g_texturedMaterialBar = TwNewBar("Textured_Materials");
	TwDefine(" Textured_Materials label='Textured Materials' size='250 240' color='216 96 96' position='1025 380' valueswidth=100");

	initTexturedMaterialBarHelper();
}

void TwGUIManager::initTexturedMaterialBarHelper()
{
	// Process materials
	const std::unordered_map<std::string, Material *> &mmMap = _mm->getMaterialMapRef();
	for (std::unordered_map<std::string, Material *>::const_iterator it = mmMap.begin();
		it != mmMap.end(); ++it)
	{
		const std::string &name = it->first;
		Material *mat = it->second;

		if (!mat->getUseTexture()) continue;

		// Moved code to a function
		addTexturedMaterialToGUI(g_texturedMaterialBar, mat, TEXTURED_MATERIAL_GROUP_NAME, _index);
	}
}

void TwGUIManager::addModelToGUI(TwBar* bar, GeometryObject* geom, std::string group, int& index) {
	std::string name = geom->getName();

	std::string nameGroupDef = " group='" + name + "' ";

	std::string indexStr = std::to_string(index);
	std::string posXVarName = POS_X_LABEL + indexStr;
	std::string posYVarName = POS_Y_LABEL + indexStr;
	std::string posZVarName = POS_Z_LABEL + indexStr;

	std::string scaleXVarName = SCALE_X_LABEL + indexStr;
	std::string scaleYVarName = SCALE_Y_LABEL + indexStr;
	std::string scaleZVarName = SCALE_Z_LABEL + indexStr;
	std::string scaleUniformVarName = SCALE_UNIFORM_LABEL + indexStr;

	std::string rotXVarName = ROT_X_LABEL + indexStr;
	std::string rotYVarName = ROT_Y_LABEL + indexStr;
	std::string rotZVarName = ROT_Z_LABEL + indexStr;

	std::string scaleLimitVal = " step=0.05";

	BarItem* item = new BarItem();
	item->bar = bar;
	item->name = name;

	std::string fitToScreenDef = nameGroupDef + " label='" + FIT_TO_SCREEN_LABEL + "'";
	TwAddButton(bar, fitToScreenDef.c_str(),
		[](void *clientData) {
		BarItem* item = static_cast<BarItem*>(clientData);
		std::string name = item->name;
		GeometryObjectManager* gm = Core::getWorldRef().getGeometryManager();
		GeometryObject *obj = gm->getGeometryObject(name);
		osg::MatrixTransform *geomRoot = obj->getRoot();
		fitObjectToScreen(geomRoot);
	}, item, fitToScreenDef.c_str());

	std::string editNameDef = nameGroupDef + " label='" + EDIT_NAME_LABEL + "'";
	TwAddVarCB(bar, editNameDef.c_str(), TW_TYPE_STDSTRING,
		[](const void *value, void *clientData) {
		BarItem* item = static_cast<BarItem*>(clientData);
		std::string oldName = item->name;

		const std::string *newName = static_cast<const std::string*>(value);

		GeometryObjectManager* gm = Core::getWorldRef().getGeometryManager();

		// Only rename if the name does not already exists
		if (gm->renameGeometry(oldName, *newName)) {
			GeometryObject* geom = gm->getGeometryObject(*newName);

			TwRemoveVar(item->bar, oldName.c_str());
			addModelToGUI(item->bar, geom, GEOM_GROUP_NAME, _index);
		}
	},
		[](void *value, void *clientData) {
		BarItem* item = static_cast<BarItem*>(clientData);
		std::string* showName = static_cast<std::string*>(value);
		TwCopyStdStringToLibrary(*showName, item->name);
	},
		item, editNameDef.c_str());

	//std::string materialVarName = "material" + indexStr;
	std::string materialDef = nameGroupDef + " label='Set Material'";
	TwAddButton(bar, materialDef.c_str(),
		[](void *clientData) {
		BarItem* item = (BarItem*)clientData;
		std::string name = item->name;

		// Get the material name
		std::string matName;
		std::cout << "Set material name: ";
		std::cin >> matName;

		MaterialManager* mm = Core::getWorldRef().getMaterialManager();
		Material* mat = mm->getMaterial(matName);
		if (!mat) {
			std::cout << "Material not found" << std::endl;
		}
		else {
			GeometryObjectManager* gm = Core::getWorldRef().getGeometryManager();

			gm->getGeometryObject(name)->setMaterial(mat);
		}
	},
		item, materialDef.c_str());


	std::string posXDef = nameGroupDef + " label='" + POS_X_LABEL + "'";
	TwAddVarCB(bar, posXVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float posX = *(const float *)value;
		osg::Vec3 oldPos = obj->getTranslate();
		osg::Vec3 newPos = osg::Vec3(posX, oldPos.y(), oldPos.z());
		obj->setTranslate(newPos);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *posX = static_cast<float *>(value);

		osg::Vec3 pos = obj->getTranslate();
		*posX = pos.x();
	},
		geom, posXDef.c_str());

	std::string posYDef = nameGroupDef + " label='" + POS_Y_LABEL + "'";
	TwAddVarCB(bar, posYVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float posY = *(const float *)value;
		osg::Vec3 oldPos = obj->getTranslate();
		osg::Vec3 newPos = osg::Vec3(oldPos.x(), posY, oldPos.z());
		obj->setTranslate(newPos);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *posY = static_cast<float *>(value);

		osg::Vec3 pos = obj->getTranslate();
		*posY = pos.y();
	},
		geom, posYDef.c_str());

	std::string posZDef = nameGroupDef + " label='" + POS_Z_LABEL + "'";
	TwAddVarCB(bar, posZVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float posZ = *(const float *)value;
		osg::Vec3 oldPos = obj->getTranslate();
		osg::Vec3 newPos = osg::Vec3(oldPos.x(), oldPos.y(), posZ);
		obj->setTranslate(newPos);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *posZ = static_cast<float *>(value);

		osg::Vec3 pos = obj->getTranslate();
		*posZ = pos.z();
	},
		geom, posZDef.c_str());


	std::string scaleXDef = nameGroupDef + " label='" + SCALE_X_LABEL + "'" + scaleLimitVal;
	TwAddVarCB(bar, scaleXVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float scaleX = *(const float *)value;
		osg::Vec3 oldScale = obj->getScale();
		osg::Vec3 newScale = osg::Vec3(scaleX, oldScale.y(), oldScale.z());
		obj->setScale(newScale);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *scaleX = static_cast<float *>(value);

		osg::Vec3 scale = obj->getScale();
		*scaleX = scale.x();
	},
		geom, scaleXDef.c_str());

	std::string scaleYDef = nameGroupDef + " label='" + SCALE_Y_LABEL + "'" + scaleLimitVal;
	TwAddVarCB(bar, scaleYVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float scaleY = *(const float *)value;
		osg::Vec3 oldScale = obj->getScale();
		osg::Vec3 newScale = osg::Vec3(oldScale.x(), scaleY, oldScale.z());
		obj->setScale(newScale);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *scaleY = static_cast<float *>(value);

		osg::Vec3 scale = obj->getScale();
		*scaleY = scale.y();
	},
		geom, scaleYDef.c_str());

	std::string scaleZDef = nameGroupDef + " label='" + SCALE_Z_LABEL + "'" + scaleLimitVal;
	TwAddVarCB(bar, scaleZVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float scaleZ = *(const float *)value;
		osg::Vec3 oldScale = obj->getScale();
		osg::Vec3 newScale = osg::Vec3(oldScale.x(), oldScale.y(), scaleZ);
		obj->setScale(newScale);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *scaleZ = static_cast<float *>(value);

		osg::Vec3 scale = obj->getScale();
		*scaleZ = scale.z();
	},
		geom, scaleZDef.c_str());

	std::string scaleUniformDef= nameGroupDef + " label='" + SCALE_UNIFORM_LABEL + "'" + scaleLimitVal;
	TwAddVarCB(bar, scaleUniformVarName.c_str(), TW_TYPE_FLOAT,
			   [](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float scaleUniform = *(const float *)value;
		osg::Vec3 oldScale = obj->getScale();
		osg::Vec3 newScale = osg::Vec3(scaleUniform, scaleUniform, scaleUniform);
		obj->setScale(newScale);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *scaleUniform = static_cast<float *>(value);

		osg::Vec3 scale = obj->getScale();
		*scaleUniform = scale.x();
	},
		geom, scaleUniformDef.c_str());

	std::string rotXDef = nameGroupDef + " label='" + ROT_X_LABEL + "'";
	TwAddVarCB(bar, rotXVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float rotX = *(const float *)value;			// Range is -pi to pi

		rotX *= PI_F / 180.0f;

		osg::Vec3 oldRot = obj->getEulerRotation();
		osg::Vec3 newRot = osg::Vec3(rotX, oldRot.y(), oldRot.z());
		obj->setRotation(newRot);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *rotX = static_cast<float *>(value);

		osg::Vec3 rot = obj->getEulerRotation();
		*rotX = rot.x() * 180.0f / PI_F;
	},
		geom, rotXDef.c_str());

	std::string rotYDef = nameGroupDef + " label='" + ROT_Y_LABEL + "'";
	TwAddVarCB(bar, rotYVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float rotY = *(const float *)value;			// Range is -pi/2 to pi/2

		rotY /= 2.0f;
		rotY *= PI_F / 180.0f;
		/*
		float bounds = PI_F / 2.0f;

		if (rotY > bounds) {
			rotY = -bounds + (rotY - bounds);
		}
		else if (rotY < -bounds) {
			rotY = bounds + (rotY - bounds);
		}*/

		osg::Vec3 oldRot = obj->getEulerRotation();
		osg::Vec3 newRot = osg::Vec3(oldRot.x(), rotY, oldRot.z());

		obj->setRotation(newRot);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *rotY = static_cast<float *>(value);

		osg::Vec3 rot = obj->getEulerRotation();
		*rotY = rot.y() * 2.0f * 180.0f / PI_F;
	},
		geom, rotYDef.c_str());

	std::string rotZDef = nameGroupDef + " label='" + ROT_Z_LABEL + "'";
	TwAddVarCB(bar, rotZVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		addGeomToUndo(obj);

		float rotZ = *(const float *)value;			// Range is -pi to pi

		rotZ *= PI_F / 180.0f;

		osg::Vec3 oldRot = obj->getEulerRotation();
		osg::Vec3 newRot = osg::Vec3(oldRot.x(), oldRot.y(), rotZ);
		obj->setRotation(newRot);

		_currChange = makeUndoRedoNode(obj);
		GeometryObjectManipulator::updateBoundingBox();
	},
		[](void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		float *rotZ = static_cast<float *>(value);

		osg::Vec3 rot = obj->getEulerRotation();
		*rotZ = rot.z() * 180.0f / PI_F;
	},
		geom, rotZDef.c_str());

	std::string rotationVarName = "rotation" + indexStr;
	std::string rotationDef = nameGroupDef + " label='rotation'";
	TwAddVarCB(bar, rotationVarName.c_str(), TW_TYPE_QUAT4F,
		[](const void *value, void *clientData) {
		GeometryObject *obj = static_cast<GeometryObject *>(clientData);
		const float *rotArr = static_cast<const float *>(value);

		// seems like ant tweak bar is nice enough to provide temp buffer for load/write
		// so that the following is safe
		osg::Quat newRot = osg::Quat(rotArr[0], rotArr[1], rotArr[2], rotArr[3]);
		obj->setRotation(newRot);

		GeometryObjectManipulator::updateBoundingBox();
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

	std::string removeDef = nameGroupDef + " label='" + REMOVE_LABEL + "'";
	TwAddButton(bar, removeDef.c_str(),
		[](void *clientData) {
		BarItem* item = (BarItem*)clientData;
		std::string name = item->name;

		// Have to detach before deleting the geometry
		GeometryObjectManipulator::detachManipulator();

		Core::getWorldRef().getGeometryManager()->deleteGeometry(name);

		TwRemoveVar(item->bar, name.c_str());

	},
		item, removeDef.c_str());

	std::string moveStr = " Main/" + name + " group='" + group + "'";
	TwDefine(moveStr.c_str());
	std::string foldedStr = " Main/" + name + " opened=false ";
	TwDefine(foldedStr.c_str());

	index++;
}

void TwGUIManager::addLightToGUI(TwBar* bar, Light* l, std::string group, int& index)
{
	std::string name = l->getName();

	std::string nameGroupDef = " group='" + name + "' ";

	std::string indexStr = std::to_string(_index);
	std::string posXVarName = POS_X_LABEL + indexStr;
	std::string posYVarName = POS_Y_LABEL + indexStr;
	std::string posZVarName = POS_Z_LABEL + indexStr;

	if (l->getLightType() == DIRECTIONAL)
	{
		DirectionalLight *dl = l->asDirectionalLight();

		std::string intensityNameDef = nameGroupDef + " label='" + INTENSITY_LABEL + "' max=30.0 min=0.0 step=0.1";
		TwAddVarCB(bar, intensityNameDef.c_str(), TW_TYPE_FLOAT,
			[](const void *data, void *clientData) {
			Light *l = (Light *)clientData;
			addLightToUndo(l);

			l->setIntensity(*(float *)data);

			_currChange = makeUndoRedoNode(l);
		}, [](void *data, void *clientData) {
			Light *l = (Light *)clientData;
			*(float *)data = l->getIntensity();
		}, dl, intensityNameDef.c_str());

		// lightDir ( to light, not from light ) 
		std::string dirToWorldVarName = "dirToWorld" + indexStr;
		std::string dirToWorldDef = nameGroupDef + " label='dirToWorld'";
		TwAddVarCB(bar, dirToWorldVarName.c_str(), TW_TYPE_DIR3F,
			[](const void *value, void *clientData) {
			DirectionalLight *dl = static_cast<DirectionalLight *>(clientData);
			addLightToUndo(dl);

			const float *arr = static_cast<const float *>(value);
			osg::Vec3 dir = osg::Vec3(arr[0], arr[1], arr[2]);
			dl->setLightToWorldDirection(dir);

			_currChange = makeUndoRedoNode(dl);
		},
			[](void *value, void *clientData) {
			DirectionalLight *dl = static_cast<DirectionalLight *>(clientData);
			const osg::Vec3 &dir = dl->getLightToWorldDirection();
			float *arr = static_cast<float *>(value);
			arr[0] = dir.x(); arr[1] = dir.y(); arr[2] = dir.z();
		},
			dl, dirToWorldDef.c_str());

		// TW_TYPE_COLOR3F
		std::string colorVarName = COLOR_LABEL + indexStr;
		std::string colorDef = nameGroupDef + " label='" + COLOR_LABEL + "'";
		TwAddVarCB(bar, colorVarName.c_str(), TW_TYPE_COLOR3F,
			[](const void *value, void *clientData) {
			DirectionalLight *dl = static_cast<DirectionalLight *>(clientData);
			addLightToUndo(dl);

			const float *arr = static_cast<const float *>(value);
			osg::Vec3 color = osg::Vec3(arr[0], arr[1], arr[2]);
			dl->setColor(color);

			_currChange = makeUndoRedoNode(dl);
		},
			[](void *value, void *clientData) {
			DirectionalLight *dl = static_cast<DirectionalLight *>(clientData);
			const osg::Vec3 &color = dl->getColor();
			float *arr = static_cast<float *>(value);
			arr[0] = color.x(); arr[1] = color.y(); arr[2] = color.z();
		}, dl, colorDef.c_str());
	}
	else if (l->getLightType() == POINTLIGHT)
	{
		PointLight *pl = l->asPointLight();

		BarItem* item = new BarItem();
		item->bar = bar;
		item->name = name;

		std::string fitToScreenDef = nameGroupDef + " label='" + FIT_TO_SCREEN_LABEL + "'";
		TwAddButton(bar, fitToScreenDef.c_str(), 
			[](void *clientData) {
			BarItem* item = static_cast<BarItem*>(clientData);
			std::string name = item->name;
			LightManager* lm = Core::getWorldRef().getLightManager();
			PointLight *l = static_cast<PointLight *>(lm->getLight(name));
			fitPointLightToScreen(l);
		}, item, fitToScreenDef.c_str());

		std::string editNameDef = nameGroupDef + " label='" + EDIT_NAME_LABEL + "'";
		TwAddVarCB(bar, editNameDef.c_str(), TW_TYPE_STDSTRING,
			[](const void *value, void *clientData) {
			BarItem* item = static_cast<BarItem*>(clientData);
			std::string oldName = item->name;

			const std::string *newName = static_cast<const std::string*>(value);

			LightManager* lm = Core::getWorldRef().getLightManager();

			// Only rename if the name does not already exists
			if (lm->renameLight(oldName, *newName)) {
				Light* light = lm->getLight(*newName);

				TwRemoveVar(item->bar, oldName.c_str());
				addLightToGUI(item->bar, light, LIGHT_GROUP_NAME, _index);
			}
		},
			[](void *value, void *clientData) {
			BarItem* item = static_cast<BarItem*>(clientData);
			std::string* showName = static_cast<std::string*>(value);
			TwCopyStdStringToLibrary(*showName, item->name);
		},
			item, editNameDef.c_str());

		std::string intensityNameDef = nameGroupDef + " label='" + INTENSITY_LABEL + "' max=30.0 min=0.0 step=0.1";
		TwAddVarCB(bar, intensityNameDef.c_str(), TW_TYPE_FLOAT, 
			[](const void *data, void *clientData) {
			Light *l = (Light *)clientData;
			addLightToUndo(l);

			l->setIntensity(*(float *)data);

			_currChange = makeUndoRedoNode(l);
		}, [](void *data, void *clientData) {
			Light *l = (Light *)clientData;
			*(float *)data = l->getIntensity();
		}, pl, intensityNameDef.c_str());

		std::string posXDef = nameGroupDef + " label='" + POS_X_LABEL + "'";
		TwAddVarCB(bar, posXVarName.c_str(), TW_TYPE_FLOAT,
			[](const void *value, void *clientData) {
			float posX = *(const float *)value;
			PointLight *pl = static_cast<PointLight *>(clientData);
			addLightToUndo(pl);

			const osg::Vec3 &oldPos = pl->getPosition();
			pl->setPosition(osg::Vec3(posX, oldPos.y(), oldPos.z()));

			_currChange = makeUndoRedoNode(pl);
		},
			[](void *value, void *clientData) {
			float *posX = static_cast<float *>(value);
			PointLight *pl = static_cast<PointLight *>(clientData);
			const osg::Vec3 &pos = pl->getPosition();
			*posX = pos.x();

		}, pl, posXDef.c_str());

		std::string posYDef = nameGroupDef + " label='" + POS_Y_LABEL + "'";
		TwAddVarCB(bar, posYVarName.c_str(), TW_TYPE_FLOAT,
			[](const void *value, void *clientData) {
			float posY = *(const float *)value;
			PointLight *pl = static_cast<PointLight *>(clientData);
			addLightToUndo(pl);

			const osg::Vec3 &oldPos = pl->getPosition();
			pl->setPosition(osg::Vec3(oldPos.x(), posY, oldPos.z()));

			_currChange = makeUndoRedoNode(pl);
		},
			[](void *value, void *clientData) {
			float *posY = static_cast<float *>(value);
			PointLight *pl = static_cast<PointLight *>(clientData);
			const osg::Vec3 &pos = pl->getPosition();
			*posY = pos.y();

		}, pl, posYDef.c_str());

		std::string posZDef = nameGroupDef + " label='" + POS_Z_LABEL + "'";
		TwAddVarCB(bar, posZVarName.c_str(), TW_TYPE_FLOAT,
			[](const void *value, void *clientData) {
			float posZ = *(const float *)value;
			PointLight *pl = static_cast<PointLight *>(clientData);
			addLightToUndo(pl);

			const osg::Vec3 &oldPos = pl->getPosition();
			pl->setPosition(osg::Vec3(oldPos.x(), oldPos.y(), posZ));

			_currChange = makeUndoRedoNode(pl);
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
		TwAddVarCB(bar, radiusVarStr.c_str(), TW_TYPE_FLOAT,
			[](const void *value, void *clientData) {
			float rad = *static_cast<const float *>(value);
			PointLight *pl = static_cast<PointLight *>(clientData);
			addLightToUndo(pl);

			pl->setRadius(rad);

			_currChange = makeUndoRedoNode(pl);
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
		std::string colorVarName = COLOR_LABEL + indexStr;
		std::string colorDef = nameGroupDef + " label='" + COLOR_LABEL + "'";
		TwAddVarCB(bar, colorVarName.c_str(), TW_TYPE_COLOR3F,
			[](const void *value, void *clientData) {
			PointLight *pl = static_cast<PointLight *>(clientData);
			addLightToUndo(pl);

			const float *arr = static_cast<const float *>(value);
			osg::Vec3 color = osg::Vec3(arr[0], arr[1], arr[2]);
			pl->setColor(color);

			_currChange = makeUndoRedoNode(pl);
		},
			[](void *value, void *clientData) {
			PointLight *pl = static_cast<PointLight *>(clientData);
			const osg::Vec3 &color = pl->getColor();
			float *arr = static_cast<float *>(value);
			arr[0] = color.x(); arr[1] = color.y(); arr[2] = color.z();
		}, pl, colorDef.c_str());

		std::string removeDef = nameGroupDef + " label='" + REMOVE_LABEL + "'";
		TwAddButton(bar, removeDef.c_str(),
			[](void *clientData) {
			BarItem* item = (BarItem*)clientData;
			std::string name = item->name;
			Core::getWorldRef().getLightManager()->deleteLight(name);

			TwRemoveVar(item->bar, name.c_str());
		},
			item, removeDef.c_str());
	}

	std::string moveStr = " Lights/" + name + " group='" + group + "'";
	TwDefine(moveStr.c_str());
	std::string foldedStr = " Lights/" + name + " opened=false ";
	TwDefine(foldedStr.c_str());

	index++;
}

void TwGUIManager::addPlainMaterialToGUI(TwBar* bar, Material* mat, std::string group, int& index)
{
	std::string name = mat->getName();
	std::string nameGroupDef = " group='" + name + "' ";

	std::string indexStr = std::to_string(index);
	std::string limitVal = " min=0 max=1 step=0.01";

	BarItem* item = new BarItem();
	item->bar = bar;
	item->name = name;

	if (mat->hasUpdateCallback())
	{
		std::string runMatUpdateCallbackDef = nameGroupDef + " label='" + RUN_MATERIAL_UPDATE_CALLBACK_LABEL + "'";
		TwAddVarCB(bar, runMatUpdateCallbackDef.c_str(), TW_TYPE_BOOL8,
			[](const void *value, void *clientData) {
			Material *m = static_cast<Material *>(clientData);
			bool res = *(bool *)value;
			if (res) m->enableMaterialUpdate();
			else m->disableMaterialUpdate();
		}, [](void *value, void *clientData) {
			*(bool *)value = static_cast<Material *>(clientData)->isMaterialUpdateEnabled();
		}, mat, runMatUpdateCallbackDef.c_str());
	}

	std::string editNameDef = nameGroupDef + " label='" + EDIT_NAME_LABEL + "'";
	TwAddVarCB(bar, editNameDef.c_str(), TW_TYPE_STDSTRING,
		[](const void *value, void *clientData) {
		BarItem* item = static_cast<BarItem*>(clientData);
		std::string oldName = item->name;

		const std::string *newName = static_cast<const std::string*>(value);

		MaterialManager* mm = Core::getWorldRef().getMaterialManager();

		// Only rename if the name does not already exists
		if (mm->renameMaterial(oldName, *newName)) {
			Material* mat = mm->getMaterial(*newName);

			TwRemoveVar(item->bar, oldName.c_str());
			addPlainMaterialToGUI(item->bar, mat, PLAIN_MATERIAL_GROUP_NAME, _index);
		}
	},
		[](void *value, void *clientData) {
		BarItem* item = static_cast<BarItem*>(clientData);
		std::string* showName = static_cast<std::string*>(value);
		TwCopyStdStringToLibrary(*showName, item->name);
	},
		item, editNameDef.c_str());

	std::string colorVarName = COLOR_LABEL + indexStr;
	std::string colorDef = nameGroupDef + " label='" + COLOR_LABEL + "'";
	TwAddVarCB(bar, colorVarName.c_str(), TW_TYPE_COLOR3F,
		[](const void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		const float *arr = static_cast<const float *>(value);
		osg::Vec3 color = osg::Vec3(arr[0], arr[1], arr[2]);
		mat->setAlbedo(color);
	},
		[](void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		const osg::Vec3 &color = mat->getAlbedo();
		float *arr = static_cast<float *>(value);
		arr[0] = color.x(); arr[1] = color.y(); arr[2] = color.z();
	}, mat, colorDef.c_str());

	std::string roughnessVarName = ROUGHNESS_LABEL + indexStr;
	std::string roughnessDef = nameGroupDef + " label='" + ROUGHNESS_LABEL + "'" + limitVal;
	TwAddVarCB(bar, roughnessVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float val = *(const float *)value;
		mat->setRoughness(val);
	},
		[](void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float *showVal = static_cast<float *>(value);

		float val = mat->getRoughness();
		*showVal = val;
	},
		mat, roughnessDef.c_str());

	std::string specularVarName = SPECULAR_LABEL + indexStr;
	std::string specularDef = nameGroupDef + " label='" + SPECULAR_LABEL + "'" + limitVal;
	TwAddVarCB(bar, specularVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float val = *(const float *)value;
		mat->setSpecular(val);
	},
		[](void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float *showVal = static_cast<float *>(value);

		float val = mat->getSpecular();
		*showVal = val;
	},
		mat, specularDef.c_str());

	std::string metallicVarName = METALLIC_LABEL + indexStr;
	std::string metallicDef = nameGroupDef + " label='" + METALLIC_LABEL + "'" + limitVal;
	TwAddVarCB(bar, metallicVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float val = *(const float *)value;
		mat->setMetallic(val);
	},
		[](void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float *showVal = static_cast<float *>(value);

		float val = mat->getMetallic();
		*showVal = val;
	},
		mat, metallicDef.c_str());

	std::string moveStr = " Plain_Materials/" + name + " group='" + group + "'";
	TwDefine(moveStr.c_str());
	std::string foldedStr = " Plain_Materials/" + name + " opened=false ";
	TwDefine(foldedStr.c_str());

	index++;
}

void TwGUIManager::addTexturedMaterialToGUI(TwBar* bar, Material* mat, std::string group, int& index)
{
	std::string name = mat->getName();
	std::string nameGroupDef = " group='" + name + "' ";

	std::string indexStr = std::to_string(index);
	std::string limitVal = " min=0 max=1 step=0.01";

	BarItem* item = new BarItem();
	item->bar = bar;
	item->name = name;

	if (mat->hasUpdateCallback())
	{
		std::string runMatUpdateCallbackDef = nameGroupDef + " label='" + RUN_MATERIAL_UPDATE_CALLBACK_LABEL + "'";
		TwAddVarCB(bar, runMatUpdateCallbackDef.c_str(), TW_TYPE_BOOL8,
			[](const void *value, void *clientData) {
			Material *m = static_cast<Material *>(clientData);
			bool res = *(bool *)value;
			if (res) m->enableMaterialUpdate();
			else m->disableMaterialUpdate();
		}, [](void *value, void *clientData) {
			*(bool *)value = static_cast<Material *>(clientData)->isMaterialUpdateEnabled();
		}, mat, runMatUpdateCallbackDef.c_str());
	}

	std::string editNameDef = nameGroupDef + " label='" + EDIT_NAME_LABEL + "'";
	TwAddVarCB(bar, editNameDef.c_str(), TW_TYPE_STDSTRING,
		[](const void *value, void *clientData) {
		BarItem* item = static_cast<BarItem*>(clientData);
		std::string oldName = item->name;

		const std::string *newName = static_cast<const std::string*>(value);

		MaterialManager* mm = Core::getWorldRef().getMaterialManager();

		// Only rename if the name does not already exists
		if (mm->renameMaterial(oldName, *newName)) {
			Material* mat = mm->getMaterial(*newName);

			TwRemoveVar(item->bar, oldName.c_str());
			addTexturedMaterialToGUI(item->bar, mat, TEXTURED_MATERIAL_GROUP_NAME, _index);
		}
	},
		[](void *value, void *clientData) {
		BarItem* item = static_cast<BarItem*>(clientData);
		std::string* showName = static_cast<std::string*>(value);
		TwCopyStdStringToLibrary(*showName, item->name);
	},
		item, editNameDef.c_str());


	// Base material parameters
	std::string colorVarName = COLOR_LABEL + indexStr;
	std::string colorDef = nameGroupDef + " label='" + COLOR_LABEL + "'";
	TwAddVarCB(bar, colorVarName.c_str(), TW_TYPE_COLOR3F,
		[](const void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		const float *arr = static_cast<const float *>(value);
		osg::Vec3 color = osg::Vec3(arr[0], arr[1], arr[2]);
		mat->setAlbedo(color);
	},
		[](void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		const osg::Vec3 &color = mat->getAlbedo();
		float *arr = static_cast<float *>(value);
		arr[0] = color.x(); arr[1] = color.y(); arr[2] = color.z();
	}, mat, colorDef.c_str());

	std::string roughnessVarName = ROUGHNESS_LABEL + indexStr;
	std::string roughnessDef = nameGroupDef + " label='" + ROUGHNESS_LABEL + "'" + limitVal;
	TwAddVarCB(bar, roughnessVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float val = *(const float *)value;
		mat->setRoughness(val);
	},
		[](void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float *showVal = static_cast<float *>(value);

		float val = mat->getRoughness();
		*showVal = val;
	},
		mat, roughnessDef.c_str());

	std::string specularVarName = SPECULAR_LABEL + indexStr;
	std::string specularDef = nameGroupDef + " label='" + SPECULAR_LABEL + "'" + limitVal;
	TwAddVarCB(bar, specularVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float val = *(const float *)value;
		mat->setSpecular(val);
	},
		[](void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float *showVal = static_cast<float *>(value);

		float val = mat->getSpecular();
		*showVal = val;
	},
		mat, specularDef.c_str());

	std::string metallicVarName = METALLIC_LABEL + indexStr;
	std::string metallicDef = nameGroupDef + " label='" + METALLIC_LABEL + "'" + limitVal;
	TwAddVarCB(bar, metallicVarName.c_str(), TW_TYPE_FLOAT,
		[](const void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float val = *(const float *)value;
		mat->setMetallic(val);
	},
		[](void *value, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		float *showVal = static_cast<float *>(value);

		float val = mat->getMetallic();
		*showVal = val;
	},
		mat, metallicDef.c_str());

	// interpolations
	std::string lerpDef = " max=1.0 min=0.0 step=0.05";
	std::string albedoLerpDef = nameGroupDef + " label='" + ALBEDO_MAP_LERP_LABEL + "'" + lerpDef;
	TwAddVarCB(bar, albedoLerpDef.c_str(), TW_TYPE_FLOAT,
		[](const void *data, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		mat->setAlbedoTexLerp(*(float *)data);
	},
		[](void *data, void *clientData) {
		*(float *)data = static_cast<Material *>(clientData)->getAlbedoTexLerp();
	},
		mat, albedoLerpDef.c_str());

	std::string roughnessLerpDef = nameGroupDef + " label='" + ROUGHNESS_MAP_LERP_LABEL + "'" + lerpDef;
	TwAddVarCB(bar, roughnessLerpDef.c_str(), TW_TYPE_FLOAT,
		[](const void *data, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		mat->setRoughnessTexLerp(*(float *)data);
	},
		[](void *data, void *clientData) {
		*(float *)data = static_cast<Material *>(clientData)->getRoughnessTexLerp();
	},
		mat, roughnessLerpDef.c_str());

	std::string metallicLerpDef = nameGroupDef + " label='" + METALLIC_MAP_LERP_LABEL + "'" + lerpDef;
	TwAddVarCB(bar, metallicLerpDef.c_str(), TW_TYPE_FLOAT,
		[](const void *data, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		mat->setMetallicTexLerp(*(float *)data);
	},
		[](void *data, void *clientData) {
		*(float *)data = static_cast<Material *>(clientData)->getMetallicTexLerp();
	},
		mat, metallicLerpDef.c_str());


	std::string normalMapLerpDef = nameGroupDef + " label='" + NORMAL_MAP_LERP_LABEL + "'" + lerpDef;
	TwAddVarCB(bar, normalMapLerpDef.c_str(), TW_TYPE_FLOAT,
		[](const void *data, void *clientData) {
		Material *mat = static_cast<Material *>(clientData);
		mat->setNormalMapLerp(*(float *)data);
	},
		[](void *data, void *clientData) {
		*(float *)data = static_cast<Material *>(clientData)->getNormalMapMapLerp();
	},
		mat, normalMapLerpDef.c_str());
	
	// Textures
	 
	// std::string albedoPathVarName = ALBEDO_PATH_LABEL + indexStr;
	std::string albedoPathDef = nameGroupDef + " label='" + ALBEDO_PATH_LABEL + "'";
	TwAddButton(bar, albedoPathDef.c_str(),
		[](void *clientData) {
		Material *mat = static_cast<Material *>(clientData);

		std::string filePath = "";
		bool validFile = openFile(filePath);

		if (validFile) {
			const size_t newsize = 4096;
			wchar_t fromPath[newsize];

			std::string fileName = getFileName(filePath);		// Get the file name (without the path)
			strToWCchar(fromPath, filePath);

			mat->setAlbedoTexturePath(filePath, mat->getMode());
		}
	},
		mat, albedoPathDef.c_str());

	//std::string roughnessPathVarName = ROUGHNESS_PATH_LABEL + indexStr;
	std::string roughnessPathDef = nameGroupDef + " label='" + ROUGHNESS_PATH_LABEL + "'";
	TwAddButton(bar, roughnessPathDef.c_str(),
		[](void *clientData) {
		Material *mat = static_cast<Material *>(clientData);

		std::string filePath = "";
		bool validFile = openFile(filePath);

		if (validFile) {
			const size_t newsize = 4096;
			wchar_t fromPath[newsize];

			std::string fileName = getFileName(filePath);		// Get the file name (without the path)
			strToWCchar(fromPath, filePath);

			mat->setRoughnessMapPath(filePath, mat->getMode());
		}
	},
		mat, roughnessPathDef.c_str());

	//std::string metallicPathVarName = METALLIC_PATH_LABEL + indexStr;
	std::string metallicPathDef = nameGroupDef + " label='" + METALLIC_PATH_LABEL + "'";
	TwAddButton(bar, metallicPathDef.c_str(),
		[](void *clientData) {
		Material *mat = static_cast<Material *>(clientData);

		std::string filePath = "";
		bool validFile = openFile(filePath);

		if (validFile) {
			const size_t newsize = 4096;
			wchar_t fromPath[newsize];

			std::string fileName = getFileName(filePath);		// Get the file name (without the path)
			strToWCchar(fromPath, filePath);

			mat->setMetallicMapPath(filePath, mat->getMode());
		}
	},
		mat, metallicPathDef.c_str());

	//std::string normalMapPathVarName = NORMAL_MAP_PATH_LABEL + indexStr;
	std::string normalMapPathDef = nameGroupDef + " label='" + NORMAL_MAP_PATH_LABEL + "'";
	TwAddButton(bar, normalMapPathDef.c_str(),
		[](void *clientData) {
		Material *mat = static_cast<Material *>(clientData);

		std::string filePath = "";
		bool validFile = openFile(filePath);

		if (validFile) {
			const size_t newsize = 4096;
			wchar_t fromPath[newsize];

			std::string fileName = getFileName(filePath);		// Get the file name (without the path)
			strToWCchar(fromPath, filePath);

			mat->setNormalMapPath(filePath, mat->getMode());
		}
	},
		mat, normalMapPathDef.c_str());

	std::string moveStr = " Textured_Materials/" + name + " group='" + group + "'";
	TwDefine(moveStr.c_str());
	std::string foldedStr = " Textured_Materials/" + name + " opened=false ";
	TwDefine(foldedStr.c_str());


	index++;
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

UndoRedoNode* TwGUIManager::makeUndoRedoNode(GeometryObject* geom)
{
	ModelInfo info = ModelInfo();
	info.name = geom->getName();
	info.matrix = geom->getMatrix();

	UndoRedoValue value = UndoRedoValue();
	value.model = info;

	UndoRedoNode* item = new UndoRedoNode();
	item->type = UndoRedoType::MODEL;
	item->value = value;

	return item;
}

UndoRedoNode* TwGUIManager::makeUndoRedoNode(Light* light)
{
	LightInfo info = LightInfo();
	info.name = light->getName();
	info.color = light->getColor();
	info.shadow = light->getCastShadow();
	info.intensity = light->getIntensity();
	
	switch (light->getLightType()) {
		case LightType::POINTLIGHT:
			info.posDir = light->asPointLight()->getPosition();
			info.radius = light->asPointLight()->getRadius();
			break;
		case LightType::DIRECTIONAL:
			info.posDir = light->asDirectionalLight()->getLightToWorldDirection();
			break;
	}

	UndoRedoValue value = UndoRedoValue();
	value.light = info;

	UndoRedoNode* item = new UndoRedoNode();
	item->type = UndoRedoType::LIGHT;
	item->value = value;

	return item;
}

void TwGUIManager::addGeomToUndo(GeometryObject* geom)
{
	UndoRedoNode* item = makeUndoRedoNode(geom);

	_undos.push_back(item);
	clearRedoList();
}

void TwGUIManager::addLightToUndo(Light* light)
{
	UndoRedoNode* item = makeUndoRedoNode(light);

	_undos.push_back(item);
	clearRedoList();
}

void TwGUIManager::clearRedoList() 
{
	int size = _redos.size();
	for (int i = 0; i < size; i++) {
		UndoRedoNode* item = _redos.back();
		_redos.pop_back();
		delete item;
	}
}

void TwGUIManager::doUndoRedo(std::vector<UndoRedoNode*> &from, std::vector<UndoRedoNode*> &dest)
{
	if (from.size() > 0) {
		UndoRedoNode* oldChange = _currChange;		// before the changes	

		GeometryObjectManager* gm = Core::getWorldRef().getGeometryManager();
		LightManager* lm = Core::getWorldRef().getLightManager();
		bool didChange = false;

		// Loop b/c previous geoms may have been deleted/renamed
		while (from.size() > 0) {
			_currChange = from.back();
			from.pop_back();

			// Check type to apply appropriate undo
			ModelInfo m_info; LightInfo l_info;
			GeometryObject* geom = NULL;
			Light* light = NULL;

			switch (_currChange->type) {
				case UndoRedoType::MODEL:
					m_info = _currChange->value.model;
					geom = gm->getGeometryObject(m_info.name);

					if (geom != NULL) {
						geom->setMatrix(m_info.matrix);
						didChange = true;
					}
					else {
						delete _currChange;
					}

					break;
				case UndoRedoType::LIGHT:
					l_info = _currChange->value.light;
					light = lm->getLight(l_info.name);

					if (light != NULL) {
						light->setColor(l_info.color);
						light->setCastShadow(l_info.shadow);
						light->setIntensity(l_info.intensity);

						switch (light->getLightType()) {
							case LightType::POINTLIGHT:
								light->asPointLight()->setPosition(l_info.posDir);
								light->asPointLight()->setRadius(l_info.radius);
								break;
							case LightType::DIRECTIONAL:
								light->asDirectionalLight()->setLightToWorldDirection(l_info.posDir);
								break;
							}
						didChange = true;
					}
					else {
						delete _currChange;
					}

					break;
			}

			if (didChange)
				break;
		}

		// Only add to redo stack if we did undo something
		if (didChange && (oldChange != NULL)) {
			dest.push_back(oldChange);
		}

		GeometryObjectManipulator::updateBoundingBox();
	}
}


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
		default:
			*(int *)&_manipulatorBits = 0x0;
		}
	}
	else
	{
		*(int *)&_manipulatorBits = 0x0;
	}

	unsigned int size = _eventsToHandle.size();
	for (unsigned int i = 0; i < size; ++i)
	{
		// attemps to fix crashing; but reason of NULL events unknown
		// actually reminds me of CSE131 return ErrorSTO back
		if (_eventsToHandle.front() == NULL)
		{
			const_cast<TwGUIManager*>(this)->_eventsToHandle.pop();
			continue;
		} 

		const osgGA::GUIEventAdapter& ea = *(_eventsToHandle.front());
		float x = ea.getX(), y = ea.getWindowHeight() - ea.getY();
		switch (ea.getEventType())
		{
		case osgGA::GUIEventAdapter::PUSH:
			TwMouseMotion(x, y);
			TwMouseButton(TW_MOUSE_PRESSED, getTwButton(ea.getButton()));
			break;

		case osgGA::GUIEventAdapter::SCROLL:
		{
			if (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP)
			{
				(*(int *)&_scrollPos) += 3;
			}
			else if (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN)
			{
				(*(int *)&_scrollPos) -= 3;
			}
			TwMouseWheel(_scrollPos);
			break;
		}

		case osgGA::GUIEventAdapter::RELEASE:
			TwMouseMotion(x, y);
			TwMouseButton(TW_MOUSE_RELEASED, getTwButton(ea.getButton()));
			break;
		case osgGA::GUIEventAdapter::DRAG:
		case osgGA::GUIEventAdapter::MOVE:
		{
			*(bool *)&_isMouseOver = (bool)TwMouseMotion(x, y);
			break;
		}
		case osgGA::GUIEventAdapter::KEYDOWN:
		{
			bool useCtrl = false;
			if (ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL)
			{
				useCtrl = true;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Tab)
			{
				if (_cameraManipulatorActive)
				{
					Core::disableCameraManipulator();
					*(bool *)&_cameraManipulatorActive = false;
				}
				else
				{
					Core::enableCameraManipulator();
					*(bool *)&_cameraManipulatorActive = true;
				}
			}
			else if (ea.getKey() == 'p')
			{
				osg::MatrixTransform *mt = NULL;
				if ((mt = GeometryObjectManipulator::getCurrNode()) != NULL)
				{
					fitObjectToScreen(mt);
				}
			}
			else if (ea.getKey() == 'c')
			{
				osg::ref_ptr<osg::MatrixTransform> obj = GeometryObjectManipulator::getCurrNode();
				if (obj != NULL) {
					std::string nodeName = obj->getName();
					std::string prefix("Transform_");

					*(std::string *)&nameToCopy = nodeName.substr(prefix.length());
				}
			}
			else if (ea.getKey() == 'v')
			{
				if (nameToCopy != "") {
					GeometryObject* newGeom = _gm->copyGeometry(nameToCopy);

					if (newGeom != NULL) {
						addModelToGUI(g_twBar, newGeom, GEOM_GROUP_NAME, _index);
					}
				}
			}
			else if (ea.getKey() == 'z')
			{
				doUndoRedo(_undos, _redos);
			}
			else if (ea.getKey() == 'y')
			{
				doUndoRedo(_redos, _undos);
			}
			else if (ea.getKey() == '`')
			{
				// TODO: currently do not detect if maximize a single bar
				if (_allBarMinimized)
				{
					maximizeAllBars();
				}
				else
				{
					minimizeAllBars();
				}
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F8)
			{
				Core::enableGameMode();
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F9)
			{
				Core::enablePassDataDisplay();
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F10)
			{
				Core::disablePassDataDisplay();
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
	case osgGA::GUIEventAdapter::FRAME:  
		return false;
	}

	// As AntTweakBar handle all events within the OpenGL context, we have to record the event here
	// and process it later in the draw callback
	if (!_freezeGUI)
	{
		_eventsToHandle.push(&ea);
	}
	return false;
}

void TwGUIManager::operator()(osg::RenderInfo& renderInfo) const
{
	osg::Viewport* viewport = renderInfo.getCurrentCamera()->getViewport();
	if (viewport) TwWindowSize(viewport->width(), viewport->height());

	updateEvents();
	TwDraw();
}

void TwGUIManager::requestFreeze()
{
	_freezeGUI = true;;
}

void TwGUIManager::requestUnFreeze()
{
	_freezeGUI = false;
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
	case osgGA::GUIEventAdapter::KEY_KP_Insert: return '0';
	case osgGA::GUIEventAdapter::KEY_KP_End: return '1';
	case osgGA::GUIEventAdapter::KEY_KP_Down: return '2';
	case osgGA::GUIEventAdapter::KEY_KP_Page_Down: return '3';
	case osgGA::GUIEventAdapter::KEY_KP_Left: return '4';
	case osgGA::GUIEventAdapter::KEY_KP_Begin: return '5';
	case osgGA::GUIEventAdapter::KEY_KP_Right: return '6';
	case osgGA::GUIEventAdapter::KEY_KP_Home: return '7';
	case osgGA::GUIEventAdapter::KEY_KP_Up: return '8';
	case osgGA::GUIEventAdapter::KEY_KP_Page_Up: return '9';
	case osgGA::GUIEventAdapter::KEY_KP_Decimal: return '.';
	case osgGA::GUIEventAdapter::KEY_KP_Delete: return TW_KEY_RETURN;

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

bool TwGUIManager::openFile(std::string &s)
{
	OPENFILENAME ofn = { sizeof ofn };
	wchar_t file[1024];
	file[0] = '\0';
	ofn.lpstrFile = file;
	ofn.nMaxFile = 1024;
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (GetOpenFileName(&ofn)) {

		std::wstring wtmp(ofn.lpstrFile);
		std::string wstr(wtmp.begin(), wtmp.end());

		s = wstr;
		return true;
	}

	return false;
}

std::string TwGUIManager::getFileName(std::string s)
{
	int pathSize = s.length();
	int index = 0;
	for (int i = 0; i < pathSize; i++) {
		char c = s.at(i);
		if (c == '\\') {
			index = i;
		}
	}

	index++;

	char fileName[4096];
	for (int i = index; i < pathSize; i++) {
		fileName[i - index] = s.at(i);
	}
	fileName[pathSize - index] = '\0';

	std::string str(fileName);
	return str;
}

void TwGUIManager::strToWCchar(wchar_t* w, std::string s)
{
	const char* tmp = s.c_str();

	size_t origsize = strlen(tmp) + 1;
	size_t convertedChars = 0;
	//const size_t newsize = 4096;
	//wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, w, origsize, tmp, _TRUNCATE);

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

std::string TwGUIManager::tagify(std::string tag, osg::Quat &q)
{
	std::string ret = "";

	for (int i = 0; i < 4; i++) {
		ret = ret + std::to_string(q[i]);

		if (i < 3) {
			ret = ret + " ";
		}
	}

	return addTags(tag, ret);
}

void TwGUIManager::fitPointLightToScreen(PointLight *l)
{
	// TODO: support fit to internal area size when area point light is implemented
	osg::Vec3 center = l->getPosition();
	float radius = 5;
	osg::Vec3 eyePos = Core::getMainCamera().getEyePosition();
	osg::Vec3 dirVec = eyePos - center;
	osg::Vec3 fromObjToEyeScale = dirVec;
	fromObjToEyeScale.normalize();
	fromObjToEyeScale *= radius;
	osg::Vec3 fromEyeToScalePoint = -dirVec + fromObjToEyeScale;
	osg::Vec3 newEyePos = eyePos + fromEyeToScalePoint;
	Core::setCurrentCameraManipulatorHomePosition(newEyePos, center, osg::Vec3(0, 0, 1));
}

void TwGUIManager::fitObjectToScreen(osg::MatrixTransform *mt)
{
	osg::BoundingSphere bs = mt->getBound();
	osg::Vec3 center = bs.center();
	float radius = bs.radius();
	osg::Vec3 eyePos = Core::getMainCamera().getEyePosition();
	osg::Vec3 dirVec = eyePos - center;
	osg::Vec3 fromObjToEyeScale = dirVec;
	fromObjToEyeScale.normalize();
	fromObjToEyeScale *= radius * 2.0;
	osg::Vec3 fromEyeToScalePoint = -dirVec + fromObjToEyeScale;
	osg::Vec3 newEyePos = eyePos + fromEyeToScalePoint;
	Core::setCurrentCameraManipulatorHomePosition(newEyePos, center, osg::Vec3(0, 0, 1));
}

void TwGUIManager::exportXML()
{
	TCHAR t_path[MAX_PATH];
	BROWSEINFO bi = { 0 };
	//bi.lpszTitle = ("All Folders Automatically Recursed.");
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		// get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, t_path);

		std::wstring w_path = t_path;
		std::string destPath(w_path.begin(), w_path.end());

		std::string source = Core::getWorldRef().getWorldPath();
		std::string copyCmd = "xcopy " + source + " " + destPath + "  /e /y /i /r";
		system(copyCmd.c_str());//"xcopy " + source.c_str() + " " + path + "  /e /y /i /r");

		//// Might wanna move this code somewhere else
		//ConfigSettings* config = ConfigSettings::config;
		//std::string str_export_material_xml = "";
		//std::string str_export_world_xml = "";
		//std::string str_mediaPath = "";
		//config->getValue(ConfigSettings::str_mediaFilePath, str_mediaPath);
		//config->getValue(ConfigSettings::str_material_xml, str_export_material_xml);
		//config->getValue(ConfigSettings::str_export_xml, str_export_world_xml);

		//// TODO: support custom file names
		//std::string exportWorldPath = str_mediaPath + str_export_world_xml;
		//std::string exportMaterialPath = str_mediaPath + str_export_material_xml;

		exportWorldXML(destPath + "\\World\\Export.xml");
		exportMaterialXML(destPath + "\\World\\Materials.xml");
	}
}

void TwGUIManager::exportWorldXML(std::string &path)
{
	// Open file to write
	int tabs = 0;
	std::ofstream f;
	f.open(path);

	// Headers
	write(f, tabs, "<?xml version=\"1.0\" encoding=\"utf - 8\"?>");
	write(f, tabs, "<world>");
	tabs++;

	MaterialManager* mm = Core::getWorldRef().getMaterialManager();
	GeometryObjectManager* gm = Core::getWorldRef().getGeometryManager();
	LightManager* lm = Core::getWorldRef().getLightManager();

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

		osg::Vec3 pos, scale;
		osg::Quat rot, so;
		// TODO: collider

		geom->decompose(pos, rot, scale, so);

		std::string matTag = "<material name=\"" + mat + "\" />";

		write(f, tabs, tagify("file", file));
		write(f, tabs, matTag);
		write(f, tabs, tagify("position", pos));
		write(f, tabs, tagify("orientation", rot));
		write(f, tabs, tagify("scale", scale));
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
		float intensity = light->getIntensity();

		write(f, tabs, tagify("color", color));
		write(f, tabs, tagify("castShadow", shadow));
		write(f, tabs, tagify("intensity", intensity));

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

void TwGUIManager::exportMaterialXML(std::string &path)
{
	// Open file to write
	int tabs = 0;
	std::ofstream f;
	f.open(path);

	// Headers
	write(f, tabs, "<?xml version=\"1.0\" encoding=\"utf - 8\"?>");
	write(f, tabs, "<materialList>");
	tabs++;

	MaterialManager* mm = Core::getWorldRef().getMaterialManager();

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

	// Footers
	tabs--;
	write(f, tabs, "</materialList>");
	f.close();			// Close file
}


void TwGUIManager::loadWorldXML()
{
	std::string filePath = "";
	bool validFile = openFile(filePath);

	if (validFile) {
		std::string fileName = getFileName(filePath);		// Get the file name (without the path)

		// Get the destination path for Material.xml
		std::string subPath = filePath.substr(0, filePath.length() - fileName.length());
		std::string matPath = subPath + "Materials.xml";

		osg::ref_ptr<TwGUIManager> gui = Core::getEditorGUI();

		// Delete items from managers
		GeometryObjectManager* gm = Core::getWorldRef().getGeometryManager();
		MaterialManager* mm = Core::getWorldRef().getMaterialManager();
		LightManager* lm = Core::getWorldRef().getLightManager();

		// Delete GeometryObjects
		std::unordered_map<std::string, GeometryObject *> geomObjMap = gm->getGeometryObjectMapRef();
		for (auto& x : geomObjMap) {
			TwRemoveVar(gui->g_twBar, x.first.c_str());
			gm->deleteGeometry(x.first);
		}

		// Delete Materials
		std::unordered_map<std::string, Material *> matMap = mm->getMaterialMapRef();
		for (auto& x : matMap) {
			bool isTexture = x.second->getUseTexture();
			if (isTexture) {
				TwRemoveVar(gui->g_texturedMaterialBar, x.first.c_str());
			}
			else {
				TwRemoveVar(gui->g_plainMaterialBar, x.first.c_str());
			}

			mm->deleteMaterial(x.first);
		}

		// Delete Lights
		std::unordered_map<std::string, Light *> lightMap = lm->getLightMapRef();
		for (auto& x : lightMap) {
			TwRemoveVar(gui->g_lightBar, x.first.c_str());
			lm ->deleteLight(x.first);
		}

		// Re-initialize stuff
		Core::getWorldRef().getGeometryCache()->clearCache();
		mm->reloadBuiltInMaterials();
		Core::loadMaterialFile(matPath);

		// Load the world file
		Core::getWorldRef().loadXMLFile(filePath);

		// Refresh GUI
		gui->initAddBarHelper();
		gui->initPlainMaterialBarHelper();
		gui->initTexturedMaterialBarHelper();
		gui->initLightBarHelper();
	}
}

bool TwGUIManager::_allBarMinimized = false;