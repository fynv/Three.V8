#pragma once

#include <cstdint>
#include <Windows.h>

class GLMain
{
public:
	GLMain(const wchar_t* title, int width, int height);
	virtual ~GLMain();

	void GetSize(int& width, int& height);

	void MainLoop();	
	void SetFramerate(float fps);
	void PostAction(void(*act)(void*), void* userData);

protected:
	virtual void idle() {}
	virtual void paint(int width, int height) {}
	virtual void mouseDown(int button, int clicks, int delta, int x, int y) {}
	virtual void mouseUp(int button, int clicks, int delta, int x, int y) {}
	virtual void mouseMove(int button, int clicks, int delta, int x, int y) {}
	virtual void mouseWheel(int button, int clicks, int delta, int x, int y) {} 

private:
	static GLMain*& from_hwnd(HWND hwnd);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HDC m_hdc;
	HGLRC m_hrc;

	uint32_t m_interval = 0;
	uint64_t m_time_scheduled;
	PTP_TIMER m_timer = nullptr;

	static void __stdcall  s_TimerCallback(PTP_CALLBACK_INSTANCE, PVOID context, PTP_TIMER timer);

};