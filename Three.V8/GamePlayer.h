#pragma once

#include "binding.h"

class GamePlayer
{
public:
	GamePlayer(V8VM* v8vm, int width, int height);
	~GamePlayer();

	void Draw(int width, int height);
	void LoadScript(const char* dir, const char* filename);

private:
	int m_width = -1;
	int m_height = -1;
	unsigned m_tex_msaa = -1;
	unsigned m_rbo_msaa = -1;
	unsigned m_fbo_msaa = -1;
	bool _update_framebuffers(int width, int height);

	V8VM* m_v8vm;
	std::unique_ptr<GameContext> m_context;
	v8::Function* m_callback_render = nullptr;

	void _unloadScript();
};

