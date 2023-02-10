#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class Camera;
class VolumeIsosurfaceModel;
class DrawIsosurface
{
public:
	struct Options
	{
		bool msaa;
		int num_directional_lights = 0;
		int num_directional_shadows = 0;
		bool has_reflection_map = false;
		bool has_environment_map = false;
		bool has_probe_grid = false;
		bool has_ambient_light = false;
		bool has_hemisphere_light = false;		
		bool has_fog = false;
	};

	DrawIsosurface(const Options& options);

	struct RenderParams
	{
		const Camera* camera;
		const VolumeIsosurfaceModel* model;
		const GLTexture2D* tex_depth;
		const Lights* lights;
		const GLDynBuffer* constant_fog;
	};

	void render(const RenderParams& params);

private:
	Options m_options;

	struct Bindings
	{
		int binding_directional_lights;
		int binding_directional_shadows;
		int location_tex_directional_shadow;
		int location_tex_reflection_map;
		int binding_environment_map;
		int binding_probe_grid;
		int binding_probes;		
		int binding_ambient_light;
		int binding_hemisphere_light;
		int binding_fog;
	};

	Bindings m_bindings;

	std::unique_ptr<GLProgram> m_prog;
};

