#pragma once

#include <memory>
#include <string>

#include "renderers/GLUtils.h"

class LightmapRenderTarget;
class ProbeGrid;
class LODProbeGrid;
class DecompressLightmap
{
public:
	DecompressLightmap(bool has_probe_grid, bool has_lod_probe_grid);

	struct RenderParams
	{
		const LightmapRenderTarget* atlas;
		GLTexture2D* probe_visibility_map;
		const ProbeGrid* probe_grid;
		const LODProbeGrid* lod_probe_grid;
		GLTexture2D* light_map;
	};

	void decompress(const RenderParams& params);

private:
	bool has_probe_grid;
	bool has_lod_probe_grid;
	std::unique_ptr<GLProgram> m_prog;
};
