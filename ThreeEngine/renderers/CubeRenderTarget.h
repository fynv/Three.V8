#pragma once

#include <memory>

class GLCubemap;
class GLRenderTarget;
class CubeImage;
class HDRCubeImage;
class CubeRenderTarget
{
public:
	CubeRenderTarget();
	~CubeRenderTarget();

	int m_width = -1;
	int m_height = -1;
	std::unique_ptr<GLCubemap> m_cube_map;
	std::unique_ptr<GLRenderTarget> m_faces[6];

	bool update_framebuffers(int width, int height);
	
	void GetCubeImage(CubeImage& image);
	void GetHDRCubeImage(HDRCubeImage& image);
	
};

