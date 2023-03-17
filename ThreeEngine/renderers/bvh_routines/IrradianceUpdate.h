#pragma once

#include <memory>
#include <string>

#include "renderers/GLUtils.h"

class BVHRenderTarget;
class ProbeRayList;
class ProbeGrid;
class LODProbeGrid;
class ProbeRenderTarget;

class IrradianceUpdate
{
public:
	IrradianceUpdate(bool is_lod_probe_grid = false, bool use_target = false);

	struct RenderParams
	{
		int id_start_probe;
		float mix_rate;
		const BVHRenderTarget* source;
		const ProbeRayList* prl;
		const ProbeGrid* probe_grid;
		const LODProbeGrid* lod_probe_grid;
		const ProbeRenderTarget* target;
	};

	void update(const RenderParams& params);

private:
	bool m_is_lod_probe_grid;
	bool m_use_target;
	std::unique_ptr<GLProgram> m_prog_sh;
	std::unique_ptr<GLProgram> m_prog_irr;
};

