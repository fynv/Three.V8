#pragma once

#pragma unmanaged
#include <cstdint>
#include <Windows.h>
#include "binding.h"
#include "GamePlayer.h"
#pragma managed

using namespace System;
using namespace System::Text;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;

namespace CLRBinding
{
	public delegate void ControlEvent(unsigned code);

	public ref class CGLControl : public Control
	{
	public:
		CGLControl();
		!CGLControl();
		~CGLControl()
		{
			if (!m_finalized)
			{
				this->!CGLControl();
				m_finalized = true;
			}
		}

		void MakeCurrent();
		void SetFramerate(float fps);

		ControlEvent^ pControlKey = nullptr;

		event ControlEvent^ ControlKey 
		{
			void add(ControlEvent^ p) 
			{
				pControlKey = static_cast<ControlEvent^> (Delegate::Combine(pControlKey, p));
			}

			void remove(ControlEvent^ p) 
			{
				pControlKey = static_cast<ControlEvent^> (Delegate::Remove(pControlKey, p));
			}

			void raise(unsigned code) 
			{
				if (pControlKey != nullptr)
					pControlKey->Invoke(code);
			}
		}


	protected:		
		virtual void OnPaint(PaintEventArgs^ e) override;
		virtual bool ProcessCmdKey(Message% msg, Keys keyData) override;

	private:
		HDC m_hdc = nullptr;
		HGLRC m_hrc = nullptr;
		unsigned m_interval = 0;
		uint64_t m_time_scheduled;
		PTP_TIMER m_timer = nullptr;
		GCHandle m_func_invalidate;
		bool m_finalized = false;
	};


	public value struct MouseEventArgs
	{
		int button;
		int clicks;
		int delta;
		int x;
		int y;
	};

	public delegate void PrintCallback(String^ str);
	public delegate String^ MessageCallback(String^ msg);

	public ref class CGamePlayer
	{
	public:
		CGamePlayer(String^ exec_path, Control^ window);
		!CGamePlayer();
		~CGamePlayer()
		{
			if (!m_finalized)
			{
				this->!CGamePlayer();
				m_finalized = true;
			}
		}

		void Draw(int width, int height);
		void LoadScript(String^ script_path, String^ resource_root);
		void UnloadScript();

		void OnMouseDown(MouseEventArgs e);
		void OnMouseUp(MouseEventArgs e);
		void OnMouseMove(MouseEventArgs e);
		void OnMouseWheel(MouseEventArgs e);
		void OnLongPress(int x, int y);

		void OnChar(int keyChar);
		void OnControlKey(unsigned code);

		void AddUserMessageHandler(String^ name, MessageCallback^ callback);

		String^ UserMessage(String^ name, String^ msg);
		String^ SendMessageToUser(String^ name, String^ msg);

		void SetPrintCallbacks(PrintCallback^ print_callback, PrintCallback^ error_callback);
		void PrintStd(String^ str);
		void PrintErr(String^ str);

	private:
		Control^ m_window = nullptr;
		GamePlayer* m_native = nullptr;
		bool m_finalized = false;
		GCHandle m_handle_this;

		Dictionary<String^, MessageCallback^>^ m_msg_dict;
		
		String^ SetCapture(String^ msg);
		String^ ReleaseCapture(String^ msg);

		PrintCallback^ m_print_callback = nullptr;
		PrintCallback^ m_error_callback = nullptr;
	};


}

