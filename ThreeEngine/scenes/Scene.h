#pragma once

#include "core/Object3D.h"
#include "lights/Lights.h"

class Background;
class EnvironmentMap;
class Fog;
class SimpleModel;
class GLTFModel;
class VolumeIsosurfaceModel;
class DirectionalLight;
class Scene : public Object3D
{
public:
	Background* background = nullptr;
	IndirectLight* indirectLight = nullptr;
	Fog* fog = nullptr;
	Lights lights;

	// pre-render
	std::vector<SimpleModel*> simple_models;
	std::vector<GLTFModel*> gltf_models;
	std::vector<VolumeIsosurfaceModel*> volume_isosurface_models;
	std::vector<DirectionalLight*> directional_lights;		

	void clear_lists()
	{
		simple_models.clear();
		gltf_models.clear();
		volume_isosurface_models.clear();
		directional_lights.clear();
	}
	
};

