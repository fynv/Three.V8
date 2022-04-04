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
protected:
	virtual void paint(int width, int height) override
	{
		m_game->Draw(width, height);
	}

private:	
	std::unique_ptr<Game> m_game;
};

int main()
{
	AppMain app;
	app.MainLoop();
	return 0;
}

