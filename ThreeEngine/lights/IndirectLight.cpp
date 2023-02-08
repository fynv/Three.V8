#include "IndirectLight.h"
#include "EnvironmentMap.h"
#include "renderers/CubeRenderTarget.h"
#include "renderers/GLSpaceProbeTarget.h"
#include "cameras/PerspectiveCamera.h"

#include "ProbeGrid.h"

IndirectLight::IndirectLight()
{
	
}

IndirectLight::~IndirectLight()
{

}

void IndirectLight::set_dynamic_map(bool on)
{
	if (dynamic_map == on) return;
	dynamic_map = on;
	if (on)
	{
		this->cube_target = std::unique_ptr<CubeRenderTarget>(new CubeRenderTarget);
		this->cube_target->update_framebuffers(128, 128);
		this->probe_target = std::unique_ptr<GLSpaceProbeTarget>(new GLSpaceProbeTarget);
		this->probe_camera = std::unique_ptr<PerspectiveCamera>(new PerspectiveCamera(90.0f, 1.0f, 0.1f, 100.0f));

		ProbeGrid* probe_grid = dynamic_cast<ProbeGrid*>(this);
		if (probe_grid == nullptr)
		{
			this->env_map = std::unique_ptr<EnvironmentMap>(new EnvironmentMap);
		}
		else
		{
			this->env_map = nullptr;
		}
	}
	else
	{
		this->cube_target = nullptr;
		this->probe_target = nullptr;
		this->env_map = nullptr;
	}
}