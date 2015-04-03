#include <iostream>

#include <Windows.h>

#include <osg/Camera>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>

#include "KeyboardEventHandler.h"
#include "PlayerNodeCallback.h"

osg::Vec3 position;

void setupCamera(osgViewer::Viewer& viewer) {
	const osg::Vec3 eye(0, -10, 0);
	const osg::Vec3 center(0, 1, 0);

	osgViewer::ViewerBase::Contexts context;
	viewer.getContexts(context, true);

	const osg::GraphicsContext::Traits* traits = context.front()->getTraits();

	if (!traits) {
		std::cerr << "Unable to detect screen resolution" << std::endl;
		exit(1);
	}

	float screenHeight = traits->height;
	float screenWidth = traits->width;

	viewer.setCameraManipulator(NULL);

	osg::Matrixf viewMat;
	viewMat.makeLookAt(eye, center, osg::Vec3(0, 0, 1));

	osg::Matrixf projMat;
	projMat.makePerspective(30, screenWidth / screenHeight, 1, 1000);

	osg::Camera *camera = viewer.getCamera();
	camera->setViewMatrix(viewMat);
	camera->setProjectionMatrix(projMat);
}

int main() {
	osgViewer::Viewer viewer;

	osg::ref_ptr<osg::Shape> player(new osg::Box);
	osg::ref_ptr<osg::ShapeDrawable> playerDrawable(new osg::ShapeDrawable(player));
	
	osg::ref_ptr<osg::Geode> playerNode(new osg::Geode);
	playerNode->addDrawable(playerDrawable);

	osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
	mt->addChild(playerNode);

	osg::Matrix m;
	m.makeTranslate(osg::Vec3(5, 0, 1));
	mt->setMatrix(m);

	mt->addUpdateCallback(new PlayerNodeCallback);

	viewer.setSceneData(mt);
	viewer.addEventHandler(new KeyboardEventHandler);

	viewer.realize();

	setupCamera(viewer); // Need to be called after viewer.realize()

	while (!viewer.done()) {
		viewer.frame();
	}
}
