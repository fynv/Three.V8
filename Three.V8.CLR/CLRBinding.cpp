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
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CGLControl::SetFramerate(float fps)
	{
		m_interval = (unsigned)(1000.0f / fps);
		m_time_scheduled = time_milli_sec();
		this->Invalidate();
	}

	void CGLControl::Pause()
	{
		m_paused = true;
	}

	void CGLControl::Resume()
	{
		m_paused = false;
		m_time_scheduled = time_milli_sec();
		this->Invalidate();
	}
	
	void CGLControl::OnPaint(PaintEventArgs^ e)
	{
		if (m_hdc != nullptr)
		{
			this->MakeCurrent();
			Control::OnPaint(e);
			SwapBuffers(m_hdc);
		}

		if (!m_paused && m_interval > 0)
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

	static std::string g_HandleUserMessage(void* pplayer, const char* cname, const char* cmsg)
	{
		GCHandle handle_player = GCHandle::FromIntPtr((IntPtr)pplayer);
		CGamePlayer^ player = (CGamePlayer^)handle_player.Target;

		String^ name = gcnew String(cname, 0, strlen(cname), Encoding::UTF8);
		String^ msg = gcnew String(cmsg, 0, strlen(cmsg), Encoding::UTF8);
		String^ ret = player->UserMessage(name, msg);

		int len_ret = Encoding::UTF8->GetByteCount(ret);
		array<unsigned char>^ bytes_ret = gcnew array<unsigned char>(len_ret + 1);
		Encoding::UTF8->GetBytes(ret, 0, ret->Length, bytes_ret, 0);
		bytes_ret[len_ret] = 0;
		const char* cret = (const char*)(void*)(Marshal::UnsafeAddrOfPinnedArrayElement(bytes_ret, 0));

		return cret;
	}

	String^ CGamePlayer::SetCapture(String^ msg)
	{
		m_window->Capture = true;
		return "";
	}

	String^ CGamePlayer::ReleaseCapture(String^ msg)
	{
		m_window->Capture = false;
		return "";
	}

	CGamePlayer::CGamePlayer(String^ exec_path, Control^ window)
	{
		const char* cpath = (const char*)(void*)Marshal::StringToHGlobalAnsi(exec_path);
		m_native = new GamePlayer(cpath, window->Width, window->Height);

		m_window = window;

		m_handle_this = GCHandle::Alloc(this, GCHandleType::Normal);
		void* pplayer = (void*)GCHandle::ToIntPtr(m_handle_this);
		m_native->SetUserMessageCallback(pplayer, g_HandleUserMessage);

		m_msg_dict = gcnew Dictionary<String^, MessageCallback^>();

		AddUserMessageHandler("setPointerCapture", gcnew MessageCallback(this, &CGamePlayer::SetCapture));
		AddUserMessageHandler("releasePointerCapture", gcnew MessageCallback(this, &CGamePlayer::ReleaseCapture));
	}

	CGamePlayer::!CGamePlayer()
	{
		if (m_handle_this.IsAllocated)
		{
			m_handle_this.Free();
		}
		delete m_native;
	}

	void CGamePlayer::Draw(int width, int height)
	{		
		m_native->Draw(width, height);
		m_native->Idle();
	}

	void CGamePlayer::LoadScript(String^ script_path, String^ resource_root)
	{
		if (resource_root == nullptr)
		{
			resource_root = Path::GetDirectoryName(script_path);
			script_path = Path::GetFileName(script_path);
		}		
		const char* cscript_path = (const char*)(void*)Marshal::StringToHGlobalAnsi(script_path);
		const char* cresource_root = (const char*)(void*)Marshal::StringToHGlobalAnsi(resource_root);
		m_native->LoadScript(cresource_root, cscript_path);
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

	void CGamePlayer::AddUserMessageHandler(String^ name, MessageCallback^ callback)
	{
		m_msg_dict[name] = callback;
	}

	String^ CGamePlayer::UserMessage(String^ name, String^ msg)
	{
		if (m_msg_dict->ContainsKey(name))
		{
			return m_msg_dict[name](msg);
		}
		return "";
	}

	String^ CGamePlayer::SendMessageToUser(String^ name, String^ msg)
	{		
		int len_name = Encoding::UTF8->GetByteCount(name);
		array<unsigned char>^ bytes_name = gcnew array<unsigned char>(len_name + 1);
		Encoding::UTF8->GetBytes(name, 0, name->Length, bytes_name, 0);
		bytes_name[len_name] = 0;
		const char* cname = (const char*)(void*)(Marshal::UnsafeAddrOfPinnedArrayElement(bytes_name, 0));

		int len_msg = Encoding::UTF8->GetByteCount(msg);
		array<unsigned char>^ bytes_msg = gcnew array<unsigned char>(len_msg + 1);
		Encoding::UTF8->GetBytes(msg, 0, msg->Length, bytes_msg, 0);
		bytes_msg[len_msg] = 0;
		const char* cmsg = (const char*)(void*)(Marshal::UnsafeAddrOfPinnedArrayElement(bytes_msg, 0));

		std::string cret = m_native->SendMessageToUser(cname, cmsg);
		return gcnew String(cret.c_str(), 0, cret.length(), Encoding::UTF8);
	}

	static void g_PrintStd(void* pplayer, const char* cstr)
	{
		GCHandle handle_player = GCHandle::FromIntPtr((IntPtr)pplayer);
		CGamePlayer^ player = (CGamePlayer^)handle_player.Target;
		String^ str = gcnew String(cstr, 0, strlen(cstr), Encoding::UTF8);
		player->PrintStd(str);
	}

	static void g_PrintErr(void* pplayer, const char* cstr)
	{
		GCHandle handle_player = GCHandle::FromIntPtr((IntPtr)pplayer);
		CGamePlayer^ player = (CGamePlayer^)handle_player.Target;
		String^ str = gcnew String(cstr, 0, strlen(cstr), Encoding::UTF8);
		player->PrintErr(str);
	}

	void CGamePlayer::SetPrintCallbacks(PrintCallback^ print_callback, PrintCallback^ error_callback)
	{
		void* pplayer = (void*)GCHandle::ToIntPtr(m_handle_this);

		m_print_callback = print_callback;
		m_error_callback = error_callback;

		m_native->SetPrintCallbacks(pplayer, g_PrintStd, g_PrintErr);
	}

	void CGamePlayer::PrintStd(String^ str)
	{
		m_print_callback(str);
	}

	void CGamePlayer::PrintErr(String^ str)
	{
		m_error_callback(str);
	}

}
