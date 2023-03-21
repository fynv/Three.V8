#include <GL/glew.h>
#include <glm.hpp>
#include "ProbeRenderTarget.h"

ProbeRenderTarget::ProbeRenderTarget()
{

}

ProbeRenderTarget::~ProbeRenderTarget()
{

}

bool ProbeRenderTarget::update_vis(int vis_pack_res)
{
	if (vis_pack_res != this->vis_pack_res)
	{
		m_tex_visibility = std::unique_ptr<GLTexture2D>(new GLTexture2D);
		glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16, vis_pack_res, vis_pack_res);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glBindTexture(GL_TEXTURE_2D, 0);

		this->vis_pack_res = vis_pack_res;
		return true;
	}
	return false;
}

bool ProbeRenderTarget::update_irr(int num_probes, int irr_pack_res)
{
	if (num_probes != this->num_probes || irr_pack_res != this->irr_pack_res)
	{
		for (int i = 0; i < 9; i++)
		{
			m_probe_bufs[i] = std::unique_ptr<GLBuffer>(new GLBuffer(num_probes * sizeof(glm::vec4), GL_SHADER_STORAGE_BUFFER));
		}		

		m_tex_irradiance = std::unique_ptr<GLTexture2D>(new GLTexture2D);
		glBindTexture(GL_TEXTURE_2D, m_tex_irradiance->tex_id);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_R11F_G11F_B10F, irr_pack_res, irr_pack_res);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glBindTexture(GL_TEXTURE_2D, 0);

		this->num_probes = num_probes;
		this->irr_pack_res = irr_pack_res;
		return true;
	}
	return false;
}

