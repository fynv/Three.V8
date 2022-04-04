#include "CLRBinding.h"

#pragma unmanaged
#include <stdio.h>
#include <GL/glew.h>
#include "utils/Utils.h"
#include "renderers/GLRenderer.h"
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
		DoubleBuffered = false;

		m_func_invalidate = GCHandle::Alloc(gcnew TimerCallback(this, &CGLControl::Invalidate), GCHandleType::Normal);
		IntPtr p_func_invalidate = GCHandle::ToIntPtr(m_func_invalidate);
		m_timer = CreateThreadpoolTimer(s_timer_callback, (void*)p_func_invalidate, nullptr);
	}

	CGLControl::!CGLControl()
	{
		if (m_hdc != nullptr)
		{
			wglMakeCurrent(m_hdc, m_hrc);
			GLRenderer::ClearCaches();
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

	void CGLControl::OnLoad(EventArgs^ e)
	{
		if (m_hdc != nullptr)
		{
			printf("The OpenGL context is being recreated. (This should not happen.)");
			wglMakeCurrent(m_hdc, m_hrc);
			GLRenderer::ClearCaches();
			wglMakeCurrent(m_hdc, NULL);
			wglDeleteContext(m_hrc);
		}

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
		UserControl::OnLoad(e);
	}

	void CGLControl::OnPaint(PaintEventArgs^ e)
	{
		if (m_hdc != nullptr)
		{
			wglMakeCurrent(m_hdc, m_hrc);
			UserControl::OnPaint(e);
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

	struct VMCallbackData
	{
		void* p_delegate;
		void* data;
	};

	static void s_vm_callback(void* ptr)
	{
		VMCallbackData* data = (VMCallbackData*)ptr;	

		GCHandle handle_callback = GCHandle::FromIntPtr((IntPtr)data->p_delegate);
		VMCallback^ callback = (VMCallback^)handle_callback.Target;		
		
		GCHandle handle_data = GCHandle::FromIntPtr((IntPtr)data->data);
		Object^ callback_data = (Object^)handle_data.Target;
		
		callback(callback_data);
	}

	CV8VM::CV8VM(String^ exec_path)
	{
		const char* path = (const char*)(void*)Marshal::StringToHGlobalAnsi(exec_path);
		m_native = new V8VM(path);
	}

	CV8VM::!CV8VM()
	{
		delete m_native;
	}

	void CV8VM::RunVM(VMCallback^ callback, Object^ data)
	{
		GCHandle handle_callback = GCHandle::Alloc(callback, GCHandleType::Normal);
		GCHandle handle_data = GCHandle::Alloc(data, GCHandleType::Normal);

		VMCallbackData callback_data;
		callback_data.p_delegate = (void*)GCHandle::ToIntPtr(handle_callback);
		callback_data.data = (void*)GCHandle::ToIntPtr(handle_data);

		m_native->RunVM(s_vm_callback, &callback_data);

		handle_data.Free();
		handle_callback.Free();
	}

	static void SetMouseCapture(void* pwin)
	{
		GCHandle handle_win = GCHandle::FromIntPtr((IntPtr)pwin);
		Control^ win = (Control^)handle_win.Target;
		win->Capture = true;
	}

	static void ReleaseMouseCapture(void* pwin)
	{
		GCHandle handle_win = GCHandle::FromIntPtr((IntPtr)pwin);
		Control^ win = (Control^)handle_win.Target;
		win->Capture = false;
	}

	CGamePlayer::CGamePlayer(CV8VM^ v8vm, Control^ window)
	{
		m_native = new GamePlayer(v8vm->native(), window->Width, window->Height);

		m_handle_win = GCHandle::Alloc(window, GCHandleType::Normal);
		WindowCalls wincalls;
		wincalls.window = (void*)GCHandle::ToIntPtr(m_handle_win);
		wincalls.SetMouseCapture = SetMouseCapture;
		wincalls.ReleaseMouseCapture = ReleaseMouseCapture;
		m_native->SetWindowCalls(wincalls);
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
	}

	void CGamePlayer::LoadScript(String^ fullpath)
	{
		String^ path = Path::GetDirectoryName(fullpath);
		String^ name = Path::GetFileName(fullpath);
		const char* cpath = (const char*)(void*)Marshal::StringToHGlobalAnsi(path);
		const char* cname = (const char*)(void*)Marshal::StringToHGlobalAnsi(name);
		m_native->LoadScript(cpath, cname);
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

}