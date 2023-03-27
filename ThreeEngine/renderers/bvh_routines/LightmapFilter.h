#pragma once

#include <memory>
#include <string>

#include "renderers/GLUtils.h"

class LightmapFilter
{
public:
	LightmapFilter();

	struct RenderParams
	{
		int width;
		int height;
		float texel_size;
		GLTexture2D* light_map_in;
		GLTexture2D* light_map_out;
		GLTexture2D* atlas_position;
	};

	void filter(const RenderParams& params);

private:
	std::unique_ptr<GLProgram> m_prog;

};


