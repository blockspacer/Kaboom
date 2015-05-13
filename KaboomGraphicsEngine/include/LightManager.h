#pragma once
#include <vector>
#include <unordered_map>
#include <osg/Vec3>

class LightVisualizer;
class Light;
class LightManager
{
public:
	LightManager();
	~LightManager();
	
	bool addDirectionalLight(const std::string &name,
		const osg::Vec3 &dirToWorld,
		const osg::Vec3 &color,
		bool castShadow);

	bool addPointLight(const std::string &name,
		const osg::Vec3 &position,
		const osg::Vec3 &color,
		float radius, 
		bool castShadow,
		float intensity = 1.0f);

	void deleteLight(const std::string &name);
	bool renameLight(const std::string &oldName, const std::string &newName);

	bool doesNameExist(const std::string &name);

	inline const std::unordered_map<std::string, Light *> getLightMapRef() const
	{
		return _lightsMap;
	}

	inline int getNumLights()
	{
		return _numLights;
	}

	osg::ref_ptr<osg::MatrixTransform> getVisualizerRoot();
	Light *getLight(const std::string &name);
	Light *getLight(int index);

private:
	std::unordered_map<std::string, Light *> _lightsMap;
	std::vector<Light *> _lights;
	int _numLights;

	LightVisualizer *_visualizer;
};

