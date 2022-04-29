#include <GL/glew.h>
#include "Game.h"
#include <models/GeometryCreator.h>
#include <loaders/GLTFLoader.h>

Game::Game(int width, int height) 
	: m_camera(45.0f, (float)width / (float)height, 0.1f, 100.0f)
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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

}

void Game::Draw(int width, int height)
{
	bool size_changed = m_width != width || m_height != height;
	if (size_changed)
	{
		m_width = width;
		m_height = height;
	}

	if (size_changed)
	{
		m_camera.aspect = (float)width / (float)height;
		m_camera.updateProjectionMatrix();
	}
	m_renderer.render(width, height, m_scene, m_camera);
	
}

