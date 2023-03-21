#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class ProbeRenderTarget
{
public:
	ProbeRenderTarget();
	~ProbeRenderTarget();

	int vis_pack_res = -1;
	int num_probes = -1;
	int irr_pack_res = -1;

	std::unique_ptr<GLTexture2D> m_tex_visibility;
	std::unique_ptr<GLBuffer> m_probe_bufs[9];
	std::unique_ptr<GLTexture2D> m_tex_irradiance;	
	

	bool update_vis(int vis_pack_res);
	bool update_irr(int num_probes, int irr_pack_res);

};



