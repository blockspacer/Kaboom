#include "ParticleEffectManager.h"
#include <osgDB/ReadFile>

#include "ExplosionEffect.h"

ParticleEffectManager::ParticleEffectManager()
	: _builtInExplsionEffectStr("__explosionEffect")
{
	// TODO: make sure depth write is disabled 
	// Since, currently depth write is not managed by osg states
	_particleGroup = new osg::Group;
	_particleGroup->setNodeMask(0x10);

	_particleUpdateHandler = new SparkUpdatingHandler;
	createBuiltInExplosionEffect();
}

ParticleEffectManager::~ParticleEffectManager()
{
	for (auto it : _particleTextures)
	{
		delete it.second;
	}

	_particleTextures.clear();

	for (auto it : _particleEffects)
	{
		delete it.second;
	}

	_particleEffects.clear();

	// TODO: clear graphics memory for textures
}

void ParticleEffectManager::createBuiltInParticleEffects()
{
	createBuiltInExplosionEffect();
}

void ParticleEffectManager::createBuiltInExplosionEffect()
{
	ExplosionEffect *explosion = new ExplosionEffect(this);
	_particleEffects.insert(std::make_pair(_builtInExplsionEffectStr, explosion));
	_particleGroup->addChild(explosion->getRoot());
	// TODO: refactor
	_particleUpdateHandler->addSpark(static_cast<SparkDrawable *>(explosion->getRoot()->getDrawable(0)));
}

ParticleEffect *ParticleEffectManager::getParticleEffect(enum BuiltInParticleEffect effect)
{
	switch (effect)
	{
	case EXPLOSION:
		return _particleEffects[_builtInExplsionEffectStr];
		break;
	default:
		OSG_WARN << "ParticleEffectManager:: invalid enum" << std::endl;
		return NULL;
	}
}

osg::ref_ptr<SparkUpdatingHandler> ParticleEffectManager::getParticleUpdateHandler()
{
	return _particleUpdateHandler;
}

ParticleTexture *ParticleEffectManager::getOrCreateParticleTexture(const std::string &path)
{
	auto it = _particleTextures.find(path);
	if (it == _particleTextures.end())
	{
		osg::Image *img = osgDB::readImageFile(path);
		if (img == NULL)
		{
			OSG_WARN << "Failed to load texture at " << path << std::endl;
		}
		ParticleTexture *tex = new ParticleTexture(img);
		_particleTextures.insert(std::make_pair(path, tex));
		return tex;
	}
	else
	{
		return it->second;
	}
}