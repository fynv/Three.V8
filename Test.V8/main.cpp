#include <cstdio>
#include <memory>
#include "GLMain.h"
#include <GL/glew.h>
#include <renderers/GLRenderer.h>
#include <GamePlayer.h>

#include "binding.h"

class AppMain : private GLMain
{
public:
	AppMain(V8VM* v8vm, int width = 1280, int height = 720)
		: GLMain(L"OpenGL", width, height)
	{
		glewInit();
		m_game_player = std::unique_ptr<GamePlayer>(new GamePlayer(v8vm, width, height));
		this->SetFramerate(60.0f);		
	}

	~AppMain()
	{
		
	}

	void LoadScript(const char* dir, const char* filename)
	{
		m_game_player->LoadScript(dir, filename);
	}

	void MainLoop()
	{
		GLMain::MainLoop();
	}

protected:
	virtual void paint(int width, int height) override
	{
		m_game_player->Draw(width, height);
	}

	virtual void mouseDown(int button, int clicks, int delta, int x, int y) override
	{
		m_game_player->OnMouseDown(button, clicks, delta, x, y);
	}

	virtual void mouseUp(int button, int clicks, int delta, int x, int y) override
	{
		m_game_player->OnMouseUp(button, clicks, delta, x, y);
	}

	virtual void mouseMove(int button, int clicks, int delta, int x, int y) override
	{
		m_game_player->OnMouseMove(button, clicks, delta, x, y);
	}

	virtual void mouseWheel(int button, int clicks, int delta, int x, int y) override
	{
		m_game_player->OnMouseWheel(button, clicks, delta, x, y);
	}

private:	
	std::unique_ptr<GamePlayer> m_game_player;
};

void v8main(void* ptr)
{
	V8VM* v8vm = (V8VM*)ptr;
	{
		AppMain app(v8vm);
		app.LoadScript("../game", "bundle.js");
		app.MainLoop();
	}
}

int main(int argc, char* argv[])
{
	V8VM v8vm(argv[0]);
	v8vm.RunVM(v8main, &v8vm);
	return 0;
}
