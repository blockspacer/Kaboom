#pragma once
#include <vector>
#include <unordered_map>
#include <osg/Vec3>

class Light;
class LightManager
{
public:
	LightManager();
	~LightManager();
	
	void addDirectionalLight(const std::string &name, 
		const osg::Vec3 &dirToWorld,
		const osg::Vec3 &color,
		bool castShadow);

	void addPointLight(const std::string &name,
		const osg::Vec3 &position,
		const osg::Vec3 &color,
		float radius, 
		bool castShadow);

	inline int getNumLights()
	{
		return _numLights;
	}

	Light *getLight(const std::string &name);
	Light *getLight(int index);

private:
	std::unordered_map<std::string, Light *> _lightsMap;
	std::vector<Light *> _lights;
	int _numLights;

};

