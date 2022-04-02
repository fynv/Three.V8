#pragma once

#pragma unmanaged
#include <cstdint>
#include <Windows.h>
#include "binding.h"
#include "GamePlayer.h"
#pragma managed

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;

namespace CLRBinding
{
	public ref class CGLControl : public UserControl
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

	protected:
		virtual void OnLoad(EventArgs^ e) override;
		virtual void OnPaint(PaintEventArgs^ e) override;

	private:
		HDC m_hdc = nullptr;
		HGLRC m_hrc = nullptr;
		unsigned m_interval = 0;
		uint64_t m_time_scheduled;
		PTP_TIMER m_timer = nullptr;
		GCHandle m_func_invalidate;
		bool m_finalized = false;
	};

	public delegate void VMCallback(Object^);
	public ref class CV8VM
	{
	public:
		V8VM* native() { return m_native;  }

		CV8VM(String^ exec_path);
		!CV8VM();
		~CV8VM()
		{
			if (!m_finalized)
			{
				this->!CV8VM();
				m_finalized = true;
			}
		}

		void RunVM(VMCallback^ callback, Object^ data);

	private:
		V8VM* m_native = nullptr;
		bool m_finalized = false;
	};

	public ref class CGamePlayer
	{
	public:
		CGamePlayer(CV8VM^ v8vm, int width, int height);
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
		void LoadScript(String^ fullpath);

	private:
		GamePlayer* m_native = nullptr;
		bool m_finalized = false;
	};


}

