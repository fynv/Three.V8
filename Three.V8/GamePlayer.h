#pragma once

#include "binding.h"

class GamePlayer
{
public:
	GamePlayer(V8VM* v8vm, int width, int height);
	~GamePlayer();

	int width() const { return m_width; }
	int height() const { return m_height; }

	void Draw(int width, int height);
	void LoadScript(const char* dir, const char* filename);

	void OnMouseDown(int button, int clicks, int delta, int x, int y);
	void OnMouseUp(int button, int clicks, int delta, int x, int y);
	void OnMouseMove(int button, int clicks, int delta, int x, int y);
	void OnMouseWheel(int button, int clicks, int delta, int x, int y);

private:
	int m_width = -1;
	int m_height = -1;
	unsigned m_tex_msaa = -1;
	unsigned m_rbo_msaa = -1;
	unsigned m_fbo_msaa = -1;
	bool _update_framebuffers(int width, int height);

	V8VM* m_v8vm;
	std::unique_ptr<GameContext> m_context;

	void _unloadScript();
};

