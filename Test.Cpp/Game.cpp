#include <GL/glew.h>
#include "Game.h"
#include <models/GeometryCreator.h>
#include <loaders/ImageLoader.h>
#include <lights/EnvironmentMapCreator.h>
#include <utils/Cube2Octa.h>
#include <stb_image_write.h>

#define ENABLE_MSAA 1

Game::Game(int width, int height) 
#if ENABLE_MSAA
	: m_render_target(true, true)
#else
	: m_render_target(false, false)
#endif
	, m_camera(45.0f, (float)width / (float)height, 0.1f, 100.0f)

{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	m_camera.position = { 0.0f, 0.0f, 7.0f };
	
	m_directional_light.intensity = 4.0;
	m_directional_light.position = { 5.0, 10.0, 5.0 };
	m_directional_light.setShadow(true, 4096, 4096);
	m_directional_light.setShadowProjection(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 50.0f);
	m_scene.add(&m_directional_light);

	/*m_bg.skyColor = {1.0f, 1.0f, 1.0f};
	m_bg.groundColor = { 0.02843f, 0.07819f, 0.0781f };
	m_scene.background = &m_bg;

	m_envLight.skyColor = { 1.0f, 1.0f, 1.0f };
	m_envLight.groundColor = { 0.02843f, 0.07819f, 0.0781f };
	m_scene.indirectLight = &m_envLight;*/

	CubeImage image;
	ImageLoader::LoadCubeFromFile(&image,
		"../game/assets/textures/sky_cube_face0.jpg",
		"../game/assets/textures/sky_cube_face1.jpg",
		"../game/assets/textures/sky_cube_face2.jpg",
		"../game/assets/textures/sky_cube_face3.jpg",
		"../game/assets/textures/sky_cube_face4.jpg",
		"../game/assets/textures/sky_cube_face5.jpg");

	m_bg.cubemap.load_memory_rgba(image.images[0].width(), image.images[0].height(),
		image.images[0].data(), image.images[1].data(), image.images[2].data(), image.images[3].data(), image.images[4].data(), image.images[5].data());
	m_scene.background = &m_bg;

	EnvironmentMapCreator creator;
	creator.Create(&image, &m_envLight);
	m_scene.indirectLight = &m_envLight;

	{
		int size = 1024;
		GLTexture2D tex_out;		
		glBindTexture(GL_TEXTURE_2D, tex_out.tex_id);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, size, size);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


		std::vector<uint8_t> rgba(size * size * 4);
		Cube2Octa convert;
		convert.convert(&m_bg.cubemap, &tex_out, size, size);
		glBindTexture(GL_TEXTURE_2D, tex_out.tex_id);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_write_png("octa.png", size, size, 4, rgba.data(), size * 4);

	}


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
	m_sphere.material.metallicFactor = 0.5f;
	m_sphere.material.roughnessFactor = 0.5f;
	m_sphere.material.update_uniform();
	m_scene.add(&m_sphere);
}

Game::~Game()
{

}

void Game::Draw(int width, int height)
{
	bool size_changed = m_render_target.update_framebuffers(width, height);

	if (size_changed)
	{
		m_camera.aspect = (float)width / (float)height;
		m_camera.updateProjectionMatrix();
	}
	
	m_renderer.render(m_scene, m_camera, m_render_target);

#if !ENABLE_MSAA
	m_render_target.blit_buffer(width, height, 0);
#endif
	
}

