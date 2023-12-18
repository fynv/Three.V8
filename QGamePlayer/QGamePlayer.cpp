#include <GL/glew.h>
#include <QFileInfo>
#include <QFileDialog>
#include <QMouseEvent>
#include <QWindow>
#include "QGamePlayer.h"

QGamePlayer::QGamePlayer()
{
	this->setFixedSize(800, 450);
	m_ui.setupUi(this);
	m_ui.glControl->SetFramerate(60.0f);

	connect(m_ui.glControl, SIGNAL(OnInit()), this, SLOT(OnInit()));
	connect(m_ui.glControl, SIGNAL(OnPaint(int, int)), this, SLOT(OnPaint(int, int)));
	connect(m_ui.btn_load, SIGNAL(clicked()), this, SLOT(BtnLoad_Click()));
	connect(m_ui.glControl, SIGNAL(OnMouseDown(QMouseEvent*)), this, SLOT(OnMouseDown(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnMouseUp(QMouseEvent*)), this, SLOT(OnMouseUp(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnMouseMove(QMouseEvent*)), this, SLOT(OnMouseMove(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnWheel(QWheelEvent*)), this, SLOT(OnWheel(QWheelEvent*)));
	connect(m_ui.glControl, SIGNAL(OnChar(int)), this, SLOT(OnChar(int)));
	connect(m_ui.glControl, SIGNAL(OnControlKey(int)), this, SLOT(OnControlKey(int)));
	connect(m_ui.btn_rotate, SIGNAL(clicked()), this, SLOT(btn_rotate_Click()));

	press_timer.setSingleShot(true);
	press_timer.setInterval(std::chrono::milliseconds(500));
	connect(&press_timer, SIGNAL(timeout()), this, SLOT(OnLongPress()));
}

QGamePlayer::~QGamePlayer()
{
	m_ui.glControl->makeCurrent();
	m_game_player = nullptr;
}

void QGamePlayer::LoadScript(QString path)
{
	QString resource_root = QFileInfo(path).path();
	QString script_filename = QFileInfo(path).fileName();
	m_game_player->LoadScript(resource_root.toLocal8Bit().data(), script_filename.toLocal8Bit().data());
}

void QGamePlayer::OnInit()
{
	int width, height;
	m_ui.glControl->GetPhysicalSize(width, height);

	QString path = QCoreApplication::applicationFilePath();
	std::string cpath = path.toLocal8Bit().toStdString();
	m_game_player = std::unique_ptr<GamePlayer>(new GamePlayer(cpath.c_str(), width, height));

	LoadScript(default_script);
}

void QGamePlayer::OnPaint(int width, int height)
{
	if (m_game_player == nullptr) return;	

	m_game_player->Draw(width, height);
	m_game_player->Idle();	
}

void QGamePlayer::BtnLoad_Click()
{
	if (m_game_player == nullptr) return;

	QString filename = QFileDialog::getOpenFileName(this, tr("Open Script"), QString(), tr("Script Files (*.js)"));
	if (!filename.isNull())
	{
		m_ui.glControl->makeCurrent();
		LoadScript(filename);
	}
}


struct MouseEventArgs
{
	int button;
	int clicks;
	int delta;
	int x;
	int y;
};


void QGamePlayer::OnLongPress()
{
	m_game_player->OnLongPress(x_down, y_down);
}

void QGamePlayer::OnMouseDown(QMouseEvent* event)
{
	if (m_game_player == nullptr) return;	
	m_ui.glControl->makeCurrent();

	int button = -1;
	if (event->button() == Qt::MouseButton::LeftButton)
	{
		button = 0;
	}
	else if (event->button() == Qt::MouseButton::MiddleButton)
	{
		button = 1;
	}
	else if (event->button() == Qt::MouseButton::RightButton)
	{
		button = 2;
	}
	else if (event->button() == Qt::MouseButton::XButton1)
	{
		button = 3;
	}
	else if (event->button() == Qt::MouseButton::XButton2)
	{
		button = 4;
	}

	QWindow* win = m_ui.glControl->windowHandle();
	double ratio = win->devicePixelRatio();
	QPointF pos = event->position();

	MouseEventArgs args;
	args.button = button;
	args.clicks = 1;
	args.delta = 0;
	args.x = pos.x() * ratio;
	args.y = pos.y() * ratio;
	m_game_player->OnMouseDown(args.button, args.clicks, args.delta, args.x, args.y);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		x_down = args.x;
		y_down = args.y;
		press_timer.start();
	}
}

void QGamePlayer::OnMouseUp(QMouseEvent* event)
{
	if (m_game_player == nullptr) return;	
	m_ui.glControl->makeCurrent();

	int button = -1;
	if (event->button() == Qt::MouseButton::LeftButton)
	{
		button = 0;
	}
	else if (event->button() == Qt::MouseButton::MiddleButton)
	{
		button = 1;
	}
	else if (event->button() == Qt::MouseButton::RightButton)
	{
		button = 2;
	}
	else if (event->button() == Qt::MouseButton::XButton1)
	{
		button = 3;
	}
	else if (event->button() == Qt::MouseButton::XButton2)
	{
		button = 4;
	}

	QWindow* win = m_ui.glControl->windowHandle();
	double ratio = win->devicePixelRatio();
	QPointF pos = event->position();

	MouseEventArgs args;
	args.button = button;
	args.clicks = 0;
	args.delta = 0;
	args.x = pos.x() * ratio;
	args.y = pos.y() * ratio;
	m_game_player->OnMouseUp(args.button, args.clicks, args.delta, args.x, args.y);

	if (press_timer.isActive())
	{
		press_timer.stop();
	}
}

void QGamePlayer::OnMouseMove(QMouseEvent* event)
{
	if (m_game_player == nullptr) return;	
	m_ui.glControl->makeCurrent();

	int button = -1;
	if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
	{
		button = 0;
	}
	else if (event->buttons().testFlag(Qt::MouseButton::MiddleButton))
	{
		button = 1;
	}
	else if (event->buttons().testFlag(Qt::MouseButton::RightButton))
	{
		button = 2;
	}
	else if (event->buttons().testFlag(Qt::MouseButton::XButton1))
	{
		button = 3;
	}
	else if (event->buttons().testFlag(Qt::MouseButton::XButton2))
	{
		button = 4;
	}

	QWindow* win = m_ui.glControl->windowHandle();
	double ratio = win->devicePixelRatio();
	QPointF pos = event->position();

	MouseEventArgs args;
	args.button = button;
	args.clicks = 0;
	args.delta = 0;
	args.x = pos.x() * ratio;
	args.y = pos.y() * ratio;
	m_game_player->OnMouseMove(args.button, args.clicks, args.delta, args.x, args.y);

	if (press_timer.isActive())
	{
		int x = args.x;
		int y = args.y;

		int dx = x - x_down;
		int dy = y - y_down;
		double dis = sqrt((double)(dx * dx) + (double)(dy * dy));
		if (dis > 3.0)
		{
			press_timer.stop();
		}
	}
}

void QGamePlayer::OnWheel(QWheelEvent* event)
{	
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();

	QWindow* win = m_ui.glControl->windowHandle();
	double ratio = win->devicePixelRatio();
	QPointF pos = event->position();

	MouseEventArgs args;
	args.button = -1;
	args.clicks = 0;
	args.delta = event->angleDelta().y();
	args.x = pos.x() * ratio;
	args.y = pos.y() * ratio;
	m_game_player->OnMouseWheel(args.button, args.clicks, args.delta, args.x, args.y);
}

void QGamePlayer::OnChar(int charCode)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->OnChar(charCode);
}

void QGamePlayer::OnControlKey(int code)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->OnControlKey(code);
}

void QGamePlayer::btn_rotate_Click()
{
	if (!is_portrait)
	{
		this->setFixedSize(500, 720);
		m_ui.glControl->setFixedSize(360, 640);
		m_ui.btn_rotate->setText(tr("To Landscape"));
		is_portrait = true;
	}
	else
	{
		this->setFixedSize(800, 450);
		m_ui.glControl->setFixedSize(640, 360);
		m_ui.btn_rotate->setText(tr("To Portrait"));
		is_portrait = false;

	}

}

