#pragma once

#include <unordered_set>

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

	std::vector<Object3D*> widgets; // for editor visualization
	void add_widget(Object3D* object);
	void remove_widget(Object3D* object);
	void clear_widgets();

	void get_bounding_box(glm::vec3& min_pos, glm::vec3& max_pos, const glm::mat4& view_matrix = glm::identity<glm::mat4>());

	std::unordered_set<int> building_set;
};

