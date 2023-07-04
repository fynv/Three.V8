#pragma once

#include <memory>
#include "lights/Lights.h"
#include "renderers/GLUtils.h"

class Camera;
class DrawFog
{
public:
	struct Options
	{
		Options()
		{
			memset(this, 0, sizeof(Options));
		}
		bool msaa = false;
		bool has_environment_map = false;
		bool has_ambient_light = false;
		bool has_hemisphere_light = false;
		bool is_reflect = false;
	};
	DrawFog(const Options& options);

	struct RenderParams
	{
		const GLTexture2D* tex_depth;
		const Camera* camera;
		const GLBuffer* constant_fog;
		const Lights* lights;
	};

	void render(const RenderParams& params);


private:
	Options m_options;
	std::unique_ptr<GLProgram> m_prog;	


};
