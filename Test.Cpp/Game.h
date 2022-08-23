#pragma once

#include <renderers/GLRenderTarget.h>
#include <renderers/GLRenderer.h>
#include <scenes/Scene.h>
#include <cameras/PerspectiveCamera.h>
#include <lights/DirectionalLight.h>
#include <lights/HemisphereLight.h>
#include <backgrounds/Background.h>

#include <models/SimpleModel.h>
#include <models/GLTFModel.h>

class Game
{
public:
	Game(int width, int height);
	~Game();

	void Draw(int width, int height);

private:
	GLRenderTarget m_render_target;

	GLRenderer m_renderer;
	Scene m_scene;
	PerspectiveCamera m_camera;
	DirectionalLight m_directional_light;
	HemisphereBackground m_bg;
	HemisphereLight m_envLight;

	SimpleModel m_box;
	SimpleModel m_sphere;

};

