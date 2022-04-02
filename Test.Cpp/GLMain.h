#pragma once

#include <cstdint>
#include <Windows.h>

class GLMain
{
public:
	GLMain(const wchar_t* title, int width, int height);
	~GLMain();

	void GetSize(int& width, int& height);

	void MainLoop();
	void SetPaintCallback(void(*paint)(int, int, void*), void* userData);
	void SetFramerate(float fps);
	void PostAction(void(*act)(void*), void* userData);

private:
	static GLMain*& from_hwnd(HWND hwnd);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HDC m_hdc;
	HGLRC m_hrc;

	void(*m_paint_callback)(int, int, void*) = nullptr;
	void* m_paint_callback_data = nullptr;

	uint32_t m_interval = 0;
	uint64_t m_time_scheduled;
	PTP_TIMER m_timer = nullptr;

	static void __stdcall  s_TimerCallback(PTP_CALLBACK_INSTANCE, PVOID context, PTP_TIMER timer);

};