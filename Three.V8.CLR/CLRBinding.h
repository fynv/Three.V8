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


	public value struct MouseEventArgs
	{
		int button;
		int clicks;
		int delta;
		int x;
		int y;
	};

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
		void LoadScript(String^ fullpath);

		void OnMouseDown(MouseEventArgs e);
		void OnMouseUp(MouseEventArgs e);
		void OnMouseMove(MouseEventArgs e);
		void OnMouseWheel(MouseEventArgs e);

	private:
		GamePlayer* m_native = nullptr;
		bool m_finalized = false;
		GCHandle m_handle_win;
	};


}

