#include "CLRBinding.h"

#pragma unmanaged
#include <unordered_set>
#include <stdio.h>
#include <GL/glew.h>
#include "utils/Utils.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma managed

using namespace System::IO;

namespace CLRBinding
{

	public delegate void TimerCallback();
	static void __stdcall s_timer_callback(PTP_CALLBACK_INSTANCE, PVOID p_func_invalidate, PTP_TIMER timer)
	{
		GCHandle handle = GCHandle::FromIntPtr((IntPtr)p_func_invalidate);
		TimerCallback^ func_invalidate = (TimerCallback^)handle.Target;
		func_invalidate();
	}

	CGLControl::CGLControl()
	{
		SetStyle(ControlStyles::Opaque, true);
		SetStyle(ControlStyles::UserPaint, true);
		SetStyle(ControlStyles::AllPaintingInWmPaint, true);		
		SetStyle(ControlStyles::Selectable, true);
		DoubleBuffered = false;
		

		m_func_invalidate = GCHandle::Alloc(gcnew TimerCallback(this, &CGLControl::Invalidate), GCHandleType::Normal);
		IntPtr p_func_invalidate = GCHandle::ToIntPtr(m_func_invalidate);
		m_timer = CreateThreadpoolTimer(s_timer_callback, (void*)p_func_invalidate, nullptr);

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

		HWND hwnd = (HWND)(void*)this->Handle;
		m_hdc = GetDC(hwnd);
		int pfm = ChoosePixelFormat(m_hdc, &pfd);
		SetPixelFormat(m_hdc, pfm, &pfd);
		m_hrc = wglCreateContext(m_hdc);
		wglMakeCurrent(m_hdc, m_hrc);
		glewInit();
	}

	CGLControl::!CGLControl()
	{
		if (m_hdc != nullptr)
		{
			wglMakeCurrent(m_hdc, NULL);
			wglDeleteContext(m_hrc);
		}

		SetThreadpoolTimer(m_timer, NULL, 0, 0);
		WaitForThreadpoolTimerCallbacks(m_timer, TRUE);
		CloseThreadpoolTimer(m_timer);
		m_func_invalidate.Free();

	}

	void CGLControl::MakeCurrent()
	{
		wglMakeCurrent(m_hdc, m_hrc);
	}

	void CGLControl::SetFramerate(float fps)
	{
		m_interval = (unsigned)(1000.0f / fps);
		m_time_scheduled = time_milli_sec();
		this->Invalidate();
	}
	
	void CGLControl::OnPaint(PaintEventArgs^ e)
	{
		if (m_hdc != nullptr)
		{
			wglMakeCurrent(m_hdc, m_hrc);
			Control::OnPaint(e);
			SwapBuffers(m_hdc);
		}

		if (m_interval > 0)
		{
			unsigned interval = m_interval;
			uint64_t t = time_milli_sec();
			int delta = (int)(int64_t)(t - m_time_scheduled);
			if (delta > interval) interval = 0;
			else if (delta > 0) interval -= delta;
			m_time_scheduled = t + interval;

			if (interval > 0)
			{
				ULARGE_INTEGER due;
				due.QuadPart = (ULONGLONG)(-((int64_t)interval * 10000LL));
				::FILETIME ft;
				ft.dwHighDateTime = due.HighPart;
				ft.dwLowDateTime = due.LowPart;
				SetThreadpoolTimer(m_timer, &ft, 0, 0);
			}
			else
			{
				this->Invalidate();
			}
		}		
	}

	bool CGLControl::ProcessCmdKey(Message% msg, Keys keyData)
	{
		static std::unordered_set<unsigned> s_set =
		{
			8, 33, 34, 35, 36, 37, 38, 39, 40, 45, 46, 
		};

		unsigned code = (unsigned)(keyData);
		if (s_set.find(code) != s_set.end())
		{
			ControlKey(code);
			return true;
		}				
		return Control::ProcessCmdKey(msg, keyData);
	}

	static void SetMouseCapture(void* pwin, const char*)
	{
		GCHandle handle_win = GCHandle::FromIntPtr((IntPtr)pwin);
		Control^ win = (Control^)handle_win.Target;
		win->Capture = true;
	}

	static void ReleaseMouseCapture(void* pwin, const char*)
	{
		GCHandle handle_win = GCHandle::FromIntPtr((IntPtr)pwin);
		Control^ win = (Control^)handle_win.Target;
		win->Capture = false;
	}

	CGamePlayer::CGamePlayer(String^ exec_path, Control^ window)
	{
		const char* cpath = (const char*)(void*)Marshal::StringToHGlobalAnsi(exec_path);
		m_native = new GamePlayer(cpath, window->Width, window->Height);

		m_handle_win = GCHandle::Alloc(window, GCHandleType::Normal);
		void* p_win = (void*)GCHandle::ToIntPtr(m_handle_win);
		m_native->AddMessageHandler("setPointerCapture", { p_win, SetMouseCapture });
		m_native->AddMessageHandler("releasePointerCapture", { p_win, ReleaseMouseCapture });
	}

	CGamePlayer::!CGamePlayer()
	{
		if (m_handle_win.IsAllocated)
		{
			m_handle_win.Free();
		}
		delete m_native;
	}

	void CGamePlayer::Draw(int width, int height)
	{		
		m_native->Draw(width, height);
		m_native->Idle();
	}

	void CGamePlayer::LoadScript(String^ fullpath)
	{
		String^ path = Path::GetDirectoryName(fullpath);
		String^ name = Path::GetFileName(fullpath);
		const char* cpath = (const char*)(void*)Marshal::StringToHGlobalAnsi(path);
		const char* cname = (const char*)(void*)Marshal::StringToHGlobalAnsi(name);
		m_native->LoadScript(cpath, cname);
	}

	void CGamePlayer::UnloadScript()
	{
		m_native->UnloadScript();
	}

	void CGamePlayer::OnMouseDown(MouseEventArgs e)
	{
		m_native->OnMouseDown(e.button, e.clicks, e.delta, e.x, e.y);
	}

	void CGamePlayer::OnMouseUp(MouseEventArgs e)
	{
		m_native->OnMouseUp(e.button, e.clicks, e.delta, e.x, e.y);
	}

	void CGamePlayer::OnMouseMove(MouseEventArgs e)
	{
		m_native->OnMouseMove(e.button, e.clicks, e.delta, e.x, e.y);
	}

	void CGamePlayer::OnMouseWheel(MouseEventArgs e)
	{
		m_native->OnMouseWheel(e.button, e.clicks, e.delta, e.x, e.y);
	}

	void CGamePlayer::OnLongPress(int x, int y)
	{
		m_native->OnLongPress(x, y);
	}

	void CGamePlayer::OnChar(int keyChar)
	{
		m_native->OnChar(keyChar);
	}

	void CGamePlayer::OnControlKey(unsigned code)
	{
		m_native->OnControlKey(code);
	}

}