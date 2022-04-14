#pragma once

#include <renderers/GLRenderer.h>
#include <scenes/Scene.h>
#include <backgrounds/Background.h>
#include <cameras/PerspectiveCamera.h>
#include <models/SimpleModel.h>
#include <models/GLTFModel.h>

class Game
{
public:
	Game(int width, int height);
	~Game();

	void Draw(int width, int height);

private:
	int m_width = -1;
	int m_height = -1;

	GLRenderer m_renderer;
	Scene m_scene;
	PerspectiveCamera m_camera;
	ColorBackground m_bg;
	SimpleModel m_box;
	SimpleModel m_sphere;

};

