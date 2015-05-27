#include "ShadowDepthCamera.h"
#include <osg/Texture>
#include <osg/Polytope>
#include "PointLight.h"
#include "DirectionalLight.h"
#include "ShadowAtlas.h"
#include "Core.h"
#include "FrustumHelper.h"

ShadowDepthCamera::ShadowDepthCamera(osg::Texture2D *shadowAtlasTex, ShadowAtlas *atlas, osg::Group *geomRoot, Light *light, int face)
	: _shadowAtlasTex(shadowAtlasTex), _atlas(atlas), _light(light), _face(face), _geomRoot(geomRoot)
{
	_id = _highest_id++;

	_shadowDepthCam = new osg::Camera();

	_shadowDepthCam->setClearMask(GL_DEPTH_BUFFER_BIT); // do not clear color buffer, or atlas will be destroyed
	_shadowDepthCam->setRenderOrder(osg::Camera::PRE_RENDER);
	_shadowDepthCam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	_shadowDepthCam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
	_shadowDepthCam->attach(osg::Camera::DEPTH_BUFFER, _shadowAtlasTex);
	_shadowDepthCam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	ShadowDepthCameraCallback *callback = new ShadowDepthCameraCallback(_atlas, _light, face);
	_updateCallback = callback;
	_shadowDepthCam->setUpdateCallback(callback);
	_shadowDepthCam->addChild(_geomRoot.get());
}

void ShadowDepthCamera::setActive(bool tf)
{
	if (tf)
	{
		_shadowDepthCam->setNodeMask(~0x0);
	}
	else
	{
		_shadowDepthCam->setNodeMask(0x0);
	}
}


osg::Vec2 ShadowDepthCamera::getAtlasPosUVCoord()
{
	auto type = _light->getLightType();
	osg::Vec2i ssCoord = osg::Vec2i();
	if (type == DIRECTIONAL)
	{
		// todo	
	}
	else if (type == POINTLIGHT)
	{
		PointLight *pl = _light->asPointLight();
		ssCoord = pl->getShadowAtlasPos(_face);
	}

	int atlasSize = _atlas->getSize();
	double x = (double)ssCoord.x() / atlasSize;
	double y = (double)ssCoord.y() / atlasSize;
	return osg::Vec2(x, y);
}

float ShadowDepthCamera::getShadowMapScaleWRTAtlas()
{
	auto type = _light->getLightType();
	int resolution = 0;
	if (type == DIRECTIONAL)
	{
		// todo	
	}
	else if (type == POINTLIGHT)
	{
		PointLight *pl = _light->asPointLight();
		resolution = pl->getShadowMapRes();
	}

	double texScale = (double)resolution / _atlas->getSize();
	return texScale;
}

// todo, move this to manager
const osg::Matrix &ShadowDepthCamera::getCurrWVP()
{
	return _updateCallback->getCurrWVP();
}

// === === === === === === === === === === ===  
ShadowDepthCameraCallback::ShadowDepthCameraCallback(ShadowAtlas *atlas, Light *light, int face)
	: _atlas(atlas), _light(light), _face(face)
{
}

void ShadowDepthCameraCallback::operator()(osg::Node *node, osg::NodeVisitor *nv)
{
	// todo: check need update

	// calc mvp
	osg::Camera *cam = static_cast<osg::Camera *>(node);
	osg::Matrix viewMat;
	osg::Matrix projMat;
	makeLightSpaceMat(viewMat, projMat);
	cam->setViewMatrix(viewMat);
	cam->setProjectionMatrix(projMat);

	// Determine render viewport
	int shadowMapIndex = -1;
	int shadowMapRes = -1;
	if (_light->getLightType() == DIRECTIONAL)
	{
		DirectionalLight *dl = _light->asDirectionalLight();
		if (dl->hasShadowMapAtlasPos(_face))
		{
			return;
		}
		shadowMapIndex = dl->getShadowMapIndex(_face);
		shadowMapRes = dl->getShadowMapRes();
	}
	else if (_light->getLightType() == POINTLIGHT)
	{
		PointLight *pl = _light->asPointLight();
		if (pl->hasShadowMapAtlasPos(_face))
		{
			return;
		}
		shadowMapIndex = pl->getShadowMapIndex(_face);
		shadowMapRes = pl->getShadowMapRes();
	}

	// todo: make sure using shadowmap index doesn't not produce conflicts or override
	osg::Vec2i atlasPos = _atlas->createTile(shadowMapIndex, shadowMapRes, shadowMapRes);
	if (atlasPos.x() < 0)
	{
		int tileSize = _atlas->getTileSize();
		setShadowMapResHelper(_light, tileSize);
		shadowMapRes = tileSize;
		
		// try again
		atlasPos = _atlas->createTile(shadowMapIndex,
			tileSize, tileSize);
		if (atlasPos.x() < 0)
		{
			OSG_WARN << "ShadowDepthCamera:: Shadow atlas full. "
				"Atlas resolution dynamic change to be implemented..." << std::endl;
		}
	}

	setShadowMapAtlasPosHelper(_light, atlasPos, _face);
	cam->setViewport(atlasPos.x(), atlasPos.y(), shadowMapRes, shadowMapRes);

	traverse(node, nv);
}

void ShadowDepthCameraCallback::setShadowMapResHelper(Light *light, int resolution)
{
	if (light->getLightType() == DIRECTIONAL)
	{
		DirectionalLight *dl = light->asDirectionalLight();
		dl->setShadowMapRes(resolution);
	}
	else if (light->getLightType() == POINTLIGHT)
	{
		PointLight *pt = light->asPointLight();
		pt->setShadowMapRes(resolution);
	}
}

void ShadowDepthCameraCallback::setShadowMapAtlasPosHelper(Light *light, const osg::Vec2i &atlasPos, int face)
{
	if (light->getLightType() == DIRECTIONAL)
	{
		DirectionalLight *dl = light->asDirectionalLight();
		dl->setShadowAtlasPos(face, atlasPos); // actually face = split, needs refactor them into base class
	}
	else if (light->getLightType() == POINTLIGHT)
	{
		PointLight *pt = light->asPointLight();
		pt->setShadowAtlasPos(face, atlasPos);
	}
}

void ShadowDepthCameraCallback::makeLightSpaceMat(osg::Matrix &view, osg::Matrix &proj)
{
	osg::Matrix projMat;
	osg::Matrix viewMat;
	if (_light->getLightType() == DIRECTIONAL)
	{
	/*	DirectionalLight *dirLight = _light->asDirectionalLight();
		projMat.makeOrtho(-10.0, 10.0, -10.0, 10.0, 5.0, 1000.0);
		osg::Vec3 wsDir = dirLight->getLightToWorldDirection();
		viewMat.makeLookAt(osg::Vec3(), wsDir, osg::Vec3(0, 0, 1));*/
		DirectionalLight *dl = _light->asDirectionalLight();
		makePSSMLightSpaceMat(dl, view, proj);
	}
	else if (_light->getLightType() == POINTLIGHT)
	{
		PointLight *pt = _light->asPointLight();
		projMat.makePerspective(95, 1.0, 0.5, pt->getRadius());
		viewMat = calcPointLightViewMat(pt->getPosition(), _face);
	}

	_currWVP = viewMat * projMat;
	view = viewMat;
	proj = projMat;
}

void ShadowDepthCameraCallback::makePSSMLightSpaceMat(DirectionalLight *dl, osg::Matrix &view, osg::Matrix &proj)
{
	FrustumHelper fdata;
	fdata.updateMatrix(&Core::getMainCamera().getViewMatrix(), &Core::getMainCamera().getProjectionMatrix());
	osg::Vec3 corners[8];

	for (int i = 0; i < 8; i++)
	{
		corners[i] = fdata.corner(i);
	}

	float prevSplitDist = _face == 0 ? Core::getMainCamera().getNearPlane() : dl->getCascadeSplitDist(_face - 1);
	float splitDist = dl->getCascadeSplitDist(_face);

	// get corners of current cascade
	for (int i = 0; i < 4; i++)
	{
		osg::Vec3 cornerRay = corners[i + 4] - corners[i];
		osg::Vec3 nearCornerRay = cornerRay * prevSplitDist;
		osg::Vec3 farCornerRay = cornerRay * splitDist;
		corners[i + 4] = corners[i] + farCornerRay;
		corners[i] = corners[i] + nearCornerRay;
	}

	osg::Vec3 ws_viewFrustumCentroid = fdata.getCenter();
	osg::Vec3 ws_lightDir = -dl->getLightToWorldDirection();
	ws_lightDir.normalize();

	// use constant z-up for stability.
	osg::Vec3 upDir = osg::Vec3(0, 0, 1);

	float bsphereRad = fdata.getBSphereRadius();
	osg::Vec3 maxExtents = osg::Vec3(bsphereRad, bsphereRad, bsphereRad);
	osg::Vec3 minExtents = -maxExtents;

	osg::Vec3 extent = maxExtents - minExtents;
	osg::Vec3 shadowCamPos = ws_viewFrustumCentroid + ws_lightDir * -minExtents.z();

	osg::Matrix shadowOrthoMat = osg::Matrixd::ortho(minExtents.x(), maxExtents.x(), minExtents.y(), maxExtents.y(), 0.0, extent.z());
	osg::Matrix shadowViewMat = osg::Matrixd::lookAt(shadowCamPos, ws_viewFrustumCentroid, upDir);
	
	osg::Matrix shadowVPMat = shadowViewMat * shadowOrthoMat;
	osg::Vec4 origin(0.0f, 0.0f, 0.0f, 1.0f);
	origin = origin * shadowVPMat;
	float shadowMapResDiv2 = dl->getShadowMapRes() / 2.0;
	origin.x() *= shadowMapResDiv2;
	origin.y() *= shadowMapResDiv2;
	
	osg::Vec4 roundedOrigin;
	roundedOrigin.x() = osg::round(origin.x());
	roundedOrigin.y() = osg::round(origin.y());
	roundedOrigin.z() = osg::round(origin.z());
	roundedOrigin.w() = osg::round(origin.w());
	
	osg::Vec4 roundOffset = roundedOrigin - origin;
	float rcp = 1.0 / shadowMapResDiv2;
	roundOffset.x() *= rcp;
	roundOffset.y() *= rcp;
	roundOffset.z() = 0.0f;
	roundOffset.w() = 0.0f;
	
	shadowOrthoMat(3, 0) += roundOffset.x();
	shadowOrthoMat(3, 1) += roundOffset.y();

	view = shadowViewMat;
	proj = shadowOrthoMat;
}

osg::Matrix ShadowDepthCameraCallback::calcPointLightViewMat(const osg::Vec3 &pos, int face)
{
	// std::cout << face << std::endl;
	static const osg::Vec3 viewDirs[6] = {
		osg::Vec3(1, 0, 0),
		osg::Vec3(-1, 0, 0),
		osg::Vec3(0, 1, 0),
		osg::Vec3(0, -1, 0),
		osg::Vec3(0, 0, 1),
		osg::Vec3(0, 0, -1)
	};

	static const osg::Vec3 upDirs[6] = {
		osg::Vec3(0, 0, 1),
		osg::Vec3(0, 0, 1),
		osg::Vec3(0, 0, 1),
		osg::Vec3(0, 0, 1),
		osg::Vec3(0, -1, 0),
		osg::Vec3(0, -1, 0),
	};

	if (face < 0 || face > 5)
	{
		return osg::Matrix();
	}

	osg::Matrix cam;
	cam.makeLookAt(pos, pos + viewDirs[face], upDirs[face]);

	return cam;
}

int ShadowDepthCamera::_highest_id = 0;