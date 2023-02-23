#pragma once


#include <memory>
#include <string>
#include "renderers/GLUtils.h"

class SSAO
{
public:
	SSAO(bool msaa);
	~SSAO();

	class Buffers
	{
	public:
		Buffers();
		~Buffers();

		int m_width = -1;
		int m_height = -1;	
		int m_quat_width = -1;
		int m_quat_height = -1;
		
		std::unique_ptr<GLTexture2D> m_tex_norm;
		std::unique_ptr<GLTexture2D> m_tex_deinterleave_depth;
		std::unique_ptr<GLTexture2D> m_tex_aoz_quat;
		std::unique_ptr<GLTexture2D> m_tex_aoz;
		std::unique_ptr<GLTexture2D> m_tex_aoz2;

		void update(int width, int height);
	};

	struct RenderParams
	{
		const Buffers* buffers;
		const GLTexture2D* depth_in;
		const GLDynBuffer* constant_camera;
	};

	void render(const RenderParams& params);

private:
	bool m_msaa;
	std::unique_ptr<GLProgram> m_prog_reconstruct_normal;
	std::unique_ptr<GLProgram> m_prog_deinterleave;
	std::unique_ptr<GLProgram> m_prog_coarse_ao;
	std::unique_ptr<GLProgram> m_prog_reinterleave;
	std::unique_ptr<GLProgram> m_prog_blur_x;
	std::unique_ptr<GLProgram> m_prog_blur_y;

	
	GLBuffer m_constant_jitters;
};

