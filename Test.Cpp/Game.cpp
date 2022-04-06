#include <GL/glew.h>
#include "Game.h"
#include <models/GeometryCreator.h>
#include <loaders/GLTFLoader.h>

Game::Game(int width, int height) 
	: m_camera(45.0f, (float)width / (float)height, 0.1f, 100.0f)
{
	m_camera.position = { 0.0f, 0.0f, 10.0f };
	m_scene.background = &m_bg;
	m_bg.color = { 0.0f, 0.52f, 1.0f };

	m_box.name = "box";
	GeometryCreator::CreateBox(&m_box.geometry, 2.0f, 2.0f, 2.0f);
	m_box.translateX(-1.5f);
	m_box.rotateOnAxis(glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)), 1.0f);
	//m_box.set_color({ 0.8f, 0.4f, 0.4f });
	m_box.texture.load_file("../game/assets/textures/uv-test-bw.png", true);
	m_scene.add(&m_box);

	m_sphere.name = "sphere";
	GeometryCreator::CreateSphere(&m_sphere.geometry, 1.0f, 32, 16);
	m_sphere.translateX(1.5f);
	//m_sphere.set_color({ 0.4f, 0.8f, 0.4f });
	m_sphere.texture.load_file("../game/assets/textures/uv-test-col.png", true);
	m_scene.add(&m_sphere);
}

Game::~Game()
{
	if (m_fbo_msaa != -1)
		glDeleteFramebuffers(1, &m_fbo_msaa);
	if (m_tex_msaa != -1)
		glDeleteTextures(1, &m_tex_msaa);
	if (m_rbo_msaa != -1)
		glDeleteRenderbuffers(1, &m_rbo_msaa);
}

void Game::Draw(int width, int height)
{
	GLint backbufId = 0;

	bool size_changed = m_width != width || m_height != height;
	if (size_changed)
	{
		if (m_fbo_msaa == -1)
		{
			glGenFramebuffers(1, &m_fbo_msaa);
			glGenTextures(1, &m_tex_msaa);
			glGenRenderbuffers(1, &m_rbo_msaa);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_SRGB8_ALPHA8, width, height, true);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo_msaa);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_msaa);

		m_width = width;
		m_height = height;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

	if (size_changed)
		{
			m_camera.aspect = (float)width / (float)height;
			m_camera.updateProjectionMatrix();
		}
		m_renderer.render(width, height, m_scene, m_camera);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, backbufId);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, backbufId);
}

