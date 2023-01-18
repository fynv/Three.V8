#include "GLMain.h"
#include "utils/Utils.h"
#include <unordered_map>
#include <windowsx.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")

struct Action
{
	void(*act)(void*);
	void* userData;
};


inline void _SetWinDpiAwareness()
{
	HINSTANCE hinstLib = LoadLibraryA("user32.dll");

	BOOL(WINAPI * pSetProcessDpiAwarenessContext)(HANDLE);
	pSetProcessDpiAwarenessContext = (decltype(pSetProcessDpiAwarenessContext))GetProcAddress(hinstLib, "SetProcessDpiAwarenessContext");
	if (pSetProcessDpiAwarenessContext != nullptr)
	{
		pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
	else
	{
#ifndef DPI_ENUMS_DECLARED
		typedef enum
		{
			PROCESS_DPI_UNAWARE = 0,
			PROCESS_SYSTEM_DPI_AWARE = 1,
			PROCESS_PER_MONITOR_DPI_AWARE = 2
		} PROCESS_DPI_AWARENESS;
#endif

		HRESULT(WINAPI * pSetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
		pSetProcessDpiAwareness = (decltype(pSetProcessDpiAwareness))GetProcAddress(hinstLib, "SetProcessDpiAwareness");
		if (pSetProcessDpiAwareness != nullptr)
		{
			pSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		}
		else
		{
			BOOL(WINAPI * pSetProcessDPIAware)(void);
			pSetProcessDPIAware = (decltype(pSetProcessDPIAware))GetProcAddress(hinstLib, "SetProcessDPIAware");
			if (pSetProcessDPIAware != nullptr)
			{
				pSetProcessDPIAware();
			}
		}
	}
}

GLMain::GLMain(const wchar_t* title, int width, int height)
{
	_SetWinDpiAwareness();

	RECT rect{ 0,0,width, height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	m_hInstance = GetModuleHandle(NULL);
	WNDCLASS wc{};
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = m_hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = L"GLMain";
	RegisterClass(&wc);

	m_hWnd = CreateWindowEx(0, L"GLMain", title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, width, height, nullptr, nullptr, m_hInstance, nullptr);
	from_hwnd(m_hWnd) = this;


	ShowWindow(m_hWnd, SW_NORMAL);

	// GL Init
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER,    // Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	m_hdc = GetDC(m_hWnd);
	int pfm = ChoosePixelFormat(m_hdc, &pfd);
	SetPixelFormat(m_hdc, pfm, &pfd);
	m_hrc = wglCreateContext(m_hdc);
	wglMakeCurrent(m_hdc, m_hrc);

	// timer
	m_timer = CreateThreadpoolTimer(s_TimerCallback, this, nullptr);
}

GLMain::~GLMain()
{
	SetThreadpoolTimer(m_timer, NULL, 0, 0);
	WaitForThreadpoolTimerCallbacks(m_timer, TRUE);
	CloseThreadpoolTimer(m_timer);
	wglMakeCurrent(m_hdc, NULL);
	wglDeleteContext(m_hrc);
	DestroyWindow(m_hWnd);
}

void GLMain::GetSize(int& width, int& height)
{
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

void GLMain::MainLoop()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		idle();
	}
}

void GLMain::SetFramerate(float fps)
{
	m_interval = (unsigned)(1000.0f / fps);
	m_time_scheduled = time_milli_sec();
	RedrawWindow(m_hWnd, nullptr, nullptr, RDW_INVALIDATE);
}

void GLMain::PostAction(void(*act)(void*), void* userData)
{
	Action* p_action = new Action{ act, userData };
	PostMessage(m_hWnd, WM_USER, 0, (LPARAM)p_action);
}

void GLMain::SetMouseCapture()
{
	SetCapture(m_hWnd);
}

void GLMain::ReleaseMouseCapture()
{
	ReleaseCapture();
}

GLMain*& GLMain::from_hwnd(HWND hwnd)
{
	static std::unordered_map<HWND, GLMain*> s_map;
	return s_map[hwnd];
}

LRESULT CALLBACK GLMain::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GLMain* self = from_hwnd(hWnd);

	switch (message)
	{
	case WM_USER:
	{
		Action* p_action = (Action*)lParam;
		p_action->act(p_action->userData);
		delete p_action;
		return 0;
	}

	case WM_PAINT:
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		self->paint(width, height);
		SwapBuffers(self->m_hdc);

		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);

		if (self->m_interval > 0)
		{
			unsigned interval = self->m_interval;
			uint64_t t = time_milli_sec();
			int delta = (int)(int64_t)(t - self->m_time_scheduled);
			if (delta > (int)interval) interval = 0;
			else if (delta > 0) interval -= delta;
			self->m_time_scheduled = t + interval;

			if (interval > 0)
			{
				ULARGE_INTEGER due;
				due.QuadPart = (ULONGLONG)(-((int64_t)interval * 10000LL));
				FILETIME ft;
				ft.dwHighDateTime = due.HighPart;
				ft.dwLowDateTime = due.LowPart;
				SetThreadpoolTimer(self->m_timer, &ft, 0, 0);
			}
			else
			{
				RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
			}
		}

		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		self->mouseDown(0, 1, 0, x, y);
		return 0;
	}
	case WM_MBUTTONDOWN:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		self->mouseDown(1, 1, 0, x, y);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		self->mouseDown(2, 1, 0, x, y);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		self->mouseUp(0, 1, 0, x, y);
		return 0;
	}
	case WM_MBUTTONUP:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		self->mouseUp(1, 1, 0, x, y);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		self->mouseUp(2, 1, 0, x, y);
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		int button = -1;
		if (wParam == MK_LBUTTON) button = 0;
		if (wParam == MK_MBUTTON) button = 1;
		if (wParam == MK_RBUTTON) button = 2;
		self->mouseMove(button, 0, 0, x, y);
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		self->mouseWheel(-1, 0, zDelta, x, y);
		return 0;
	}

	case WM_CHAR:
	{
		printf("%x\n", wParam);
		return 0;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void GLMain::s_TimerCallback(PTP_CALLBACK_INSTANCE, PVOID context, PTP_TIMER timer)
{
	GLMain* self = (GLMain*)context;
	RedrawWindow(self->m_hWnd, nullptr, nullptr, RDW_INVALIDATE);
}