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
	AppMain(const char* exec_path, int width = 1280, int height = 720)
		: GLMain(L"OpenGL", width, height)
	{
		glewInit();
		m_game_player = std::unique_ptr<GamePlayer>(new GamePlayer(exec_path, width, height));
		m_game_player->AddMessageHandler("setPointerCapture", { this, s_SetMouseCapture });
		m_game_player->AddMessageHandler("releasePointerCapture", { this, s_ReleaseMouseCapture });
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

	static void s_SetMouseCapture(void* pwin, const char*)
	{
		AppMain* win = (AppMain*)pwin;
		win->SetMouseCapture();
	}

	static void s_ReleaseMouseCapture(void* pwin, const char*)
	{
		AppMain* win = (AppMain*)pwin;
		win->ReleaseMouseCapture();
	}
	
protected:
	virtual void idle() override
	{
		m_game_player->Idle();
	}

	virtual void paint(int width, int height) override
	{
		if (width > 0 && height > 0)
		{
			m_game_player->Draw(width, height);
		}
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

int main(int argc, const char* argv[])
{
	AppMain app(argv[0]);
	app.LoadScript("../game", "bundle_game.js");
	app.MainLoop();
	return 0;
}
