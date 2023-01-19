#pragma once
#include <memory>

class GLTexture2D;
class AVCPlayer
{
public:
	AVCPlayer();
	~AVCPlayer();

	void AddPacket(size_t size, const uint8_t* data);

	int width() const { return m_width;  }
	int height() const { return m_height; }
	GLTexture2D* get_texture();
	void update_texture();

private:
	class Internal;
	std::unique_ptr<Internal> m_internal;

	int m_width = 640;
	int m_height = 480;
	std::unique_ptr<GLTexture2D> m_tex;
};

