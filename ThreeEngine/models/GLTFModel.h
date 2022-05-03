#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include "core/Object3D.h"
#include "renderers/GLUtils.h"
#include "materials/MeshStandardMaterial.h"
#include "models/ModelComponents.h"
#include "models/Animation.h"

class Mesh;
class GLTFModel : public Object3D
{
public:
	std::vector<std::unique_ptr<GLTexture2D>> m_textures;
	std::vector<std::unique_ptr<MeshStandardMaterial>> m_materials;
	
	std::vector<Mesh> m_meshs;
	std::unordered_map<std::string, int> m_mesh_dict;
	void updateMeshConstants();

	std::vector<Node> m_nodes;
	std::unordered_map<std::string, int> m_node_dict;	
	std::vector<int> m_roots;	
	std::vector<Skin> m_skins;	
	bool needUpdateSkinnedMeshes = false;
	void updateNodes();

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

	void set_toon_shading(int mode, float wire_width);
};
