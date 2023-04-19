#include <GL/glew.h>
#include <QFileInfo>
#include <QFileDialog>
#include <QMouseEvent>
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
	int width = m_ui.glControl->width();
	int height = m_ui.glControl->width();

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

	QFileDialog openFileDialog(this);
	openFileDialog.setNameFilter(tr("Script Files (*.js)"));
	if (openFileDialog.exec())
	{
		m_ui.glControl->makeCurrent();
		LoadScript(openFileDialog.selectedFiles()[0]);
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

inline MouseEventArgs convert_args(QMouseEvent* event)
{
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

	MouseEventArgs args;
	args.button = button;
	args.clicks = event->type() == QMouseEvent::MouseButtonPress ? 1: 0;
	args.delta = 0;
	args.x = event->x();
	args.y = event->y();
	return args;

}

void QGamePlayer::OnLongPress()
{
	m_game_player->OnLongPress(x_down, y_down);
}

void QGamePlayer::OnMouseDown(QMouseEvent* event)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->setFocus();
	m_ui.glControl->makeCurrent();
	MouseEventArgs args = convert_args(event);		
	m_game_player->OnMouseDown(args.button, args.clicks, args.delta, args.x, args.y);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		x_down = event->x();
		y_down = event->y();
		press_timer.start();
	}
}

void QGamePlayer::OnMouseUp(QMouseEvent* event)
{
	if (m_game_player == nullptr) return;	
	m_ui.glControl->makeCurrent();
	MouseEventArgs args = convert_args(event);	
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
	MouseEventArgs args = convert_args(event);	
	m_game_player->OnMouseMove(args.button, args.clicks, args.delta, args.x, args.y);

	if (press_timer.isActive())
	{
		int x = event->x();
		int y = event->y();

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

	MouseEventArgs args;
	args.button = -1;
	args.clicks = 0;
	args.delta = event->angleDelta().y();
	args.x = qRound(event->position().x());
	args.y = qRound(event->position().y());
	m_game_player->OnMouseWheel(args.button, args.clicks, args.delta, args.x, args.y);
}