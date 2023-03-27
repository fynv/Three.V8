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

class LightmapRenderTarget;
class GLRenderer;
class Mesh;
class GLTFModel : public Object3D
{
public:
	GLTFModel();
	~GLTFModel();

	glm::vec3 m_min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
	glm::vec3 m_max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	void calculate_bounding_box();

	std::vector<std::unique_ptr<GLTexture2D>> m_textures;
	std::unordered_map<int, GLTexture2D*> m_repl_textures;
	std::unordered_map<std::string, int> m_tex_dict;
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

	bool m_show_skeleton = false;

	// Batching primitives of the same material - for BVH rendering
	std::unique_ptr<Mesh> batched_mesh;
	void batch_primitives();

	std::vector<std::vector<int>> batch_map;

	std::unique_ptr<Lightmap> lightmap;
	std::unique_ptr<LightmapRenderTarget> lightmap_target;
	void init_lightmap(GLRenderer* renderer, int width, int height, int texels_per_unit = 128);

	// animations
	void setAnimationFrame(const AnimationFrame& frame);

	std::vector<AnimationClip> m_animations;
	std::unordered_map<std::string, int> m_animation_dict;

	void buildAnimDict();

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

	void set_toon_shading(int mode, float wire_width, const glm::vec3& wire_color);
};
