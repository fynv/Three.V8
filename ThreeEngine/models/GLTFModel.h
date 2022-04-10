#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include "models/Model.h"
#include "renderers/GLUtils.h"
#include "materials/MeshStandardMaterial.h"
#include "models/ModelComponents.h"
#include "models/Animation.h"

class Mesh;
class GLTFModel : public Model
{
public:
	std::vector<std::unique_ptr<GLTexture2D>> m_textures;
	std::vector<std::unique_ptr<MeshStandardMaterial>> m_materials;
	std::vector<Mesh> m_meshs;
	std::unordered_map<std::string, int> m_mesh_dict;

	// animations
	void setAnimationFrame(const AnimationFrame& frame);

	std::vector<AnimationClip> m_animations;
	std::unordered_map<std::string, int> m_animation_dict;

	void addAnimation(const AnimationClip& anim);

	// animation play list
	struct PlayBack
	{
		int id_anim;
		double time_start;
	};
	std::vector<PlayBack> m_current_playing;

	void playAnimation(const char* name);
	void stopAnimation(const char* name);

	void updateAnimation();


};
