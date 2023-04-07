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
		this->cube_target->update_framebuffers(256, 256);
		this->probe_target = std::unique_ptr<GLSpaceProbeTarget>(new GLSpaceProbeTarget);
		this->probe_camera = std::unique_ptr<PerspectiveCamera>(new PerspectiveCamera(90.0f, 1.0f, 0.1f, 100.0f));	
	}
	else
	{
		this->cube_target = nullptr;
		this->probe_target = nullptr;
	}
}