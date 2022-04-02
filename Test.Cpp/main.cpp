#include <cstdio>
#include <memory>
#include "GLMain.h"
#include <GL/glew.h>
#include "Game.h"

class AppMain : private GLMain
{
public:
	AppMain(int width = 1280, int height = 720)
		: GLMain(L"OpenGL", width, height)		
	{
		glewInit();
		m_game = std::unique_ptr<Game>(new Game(width, height));
		this->SetFramerate(60.0f);
		this->SetPaintCallback(s_paint, this);
	}

	~AppMain()
	{
		m_game = nullptr;
		GLRenderer::ClearCaches();
	}

	void MainLoop()
	{
		GLMain::MainLoop();
	}

private:
	static void s_paint(int width, int height, void* ptr)
	{
		AppMain* self = (AppMain*)ptr;
		self->m_game->Draw(width, height);
	}
	std::unique_ptr<Game> m_game;
};

int main()
{
	AppMain app;
	app.MainLoop();
	return 0;
}

