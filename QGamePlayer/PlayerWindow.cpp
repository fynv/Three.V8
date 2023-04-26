#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QMouseEvent>
#include "PlayerWindow.h"

PlayerWindow::PlayerWindow(const char* path_proj, int idx)
{
	m_ui.setupUi(this);
	m_ui.glControl->SetFramerate(60.0f);
	m_ui.consoleTextEdit->addAction(m_ui.actionClearConsole);	
	
	QString filename = QString::fromLocal8Bit(path_proj);
	QFile file;
	file.setFileName(filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString text = file.readAll();
	file.close();

	QJsonDocument json_doc = QJsonDocument::fromJson(text.toUtf8());
	QJsonObject obj_proj = json_doc.object();
	QJsonObject obj_target = obj_proj["targets"].toArray()[idx].toObject();
	QString name = obj_target["name"].toString();
	QString output = obj_target["output"].toString();

	this->setWindowTitle(name);

	QString dir_proj = QFileInfo(filename).path();
	m_script_path = QFileInfo(dir_proj + "/" + output).absoluteFilePath();
	
	if (obj_target.contains("width") && obj_target.contains("height"))
	{
		int width = obj_target["width"].toInt();
		int height = obj_target["height"].toInt();

		this->resize(width, height);
	}

	QList<int> sizes(2);
	sizes[0] = this->height();
	sizes[1] = 0;
	m_ui.splitter->setSizes(sizes);

	connect(m_ui.glControl, SIGNAL(OnInit()), this, SLOT(OnInit()));
	connect(m_ui.glControl, SIGNAL(OnPaint(int, int)), this, SLOT(OnPaint(int, int)));
	connect(m_ui.glControl, SIGNAL(OnMouseDown(QMouseEvent*)), this, SLOT(OnMouseDown(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnMouseUp(QMouseEvent*)), this, SLOT(OnMouseUp(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnMouseMove(QMouseEvent*)), this, SLOT(OnMouseMove(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnWheel(QWheelEvent*)), this, SLOT(OnWheel(QWheelEvent*)));
	connect(m_ui.glControl, SIGNAL(OnChar(int)), this, SLOT(OnChar(int)));
	connect(m_ui.glControl, SIGNAL(OnControlKey(int)), this, SLOT(OnControlKey(int)));	

	press_timer.setSingleShot(true);
	press_timer.setInterval(std::chrono::milliseconds(500));
	connect(&press_timer, SIGNAL(timeout()), this, SLOT(OnLongPress()));

	connect(m_ui.actionClearConsole, SIGNAL(triggered()), this, SLOT(OnClearConsole()));
}

PlayerWindow::~PlayerWindow()
{
	m_ui.glControl->makeCurrent();
	m_game_player = nullptr;
}

void PlayerWindow::LoadScript(QString path)
{
	QString resource_root = QFileInfo(path).path();
	QString script_filename = QFileInfo(path).fileName();
	m_game_player->LoadScript(resource_root.toLocal8Bit().data(), script_filename.toLocal8Bit().data());
}


void PlayerWindow::print_std(void* p_self, const char* cstr)
{
	PlayerWindow* self = (PlayerWindow*)p_self;
	QString text = QString::fromUtf8(cstr);	
	self->m_ui.consoleTextEdit->setTextColor(QColor::fromRgbF(0.0f, 0.0f, 0.0f));
	self->m_ui.consoleTextEdit->append(text);

}

void PlayerWindow::err_std(void* p_self, const char* cstr)
{
	PlayerWindow* self = (PlayerWindow*)p_self;
	QString text = QString::fromUtf8(cstr);
	self->m_ui.consoleTextEdit->setTextColor(QColor::fromRgbF(1.0f, 0.0f, 0.0f));
	self->m_ui.consoleTextEdit->append(text);
}

void PlayerWindow::OnInit()
{
	int width = m_ui.glControl->width();
	int height = m_ui.glControl->width();

	QString path = QCoreApplication::applicationFilePath();
	std::string cpath = path.toLocal8Bit().toStdString();
	m_game_player = std::unique_ptr<GamePlayer>(new GamePlayer(cpath.c_str(), width, height));
	m_game_player->SetPrintCallbacks(this, print_std, err_std);

	LoadScript(m_script_path);
}

void PlayerWindow::OnPaint(int width, int height)
{
	if (m_game_player == nullptr) return;

	m_game_player->Draw(width, height);
	m_game_player->Idle();
}


struct MouseEventArgs
{
	int button;
	int clicks;
	int delta;
	int x;
	int y;
};


void PlayerWindow::OnLongPress()
{
	m_game_player->OnLongPress(x_down, y_down);
}

void PlayerWindow::OnMouseDown(QMouseEvent* event)
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

	MouseEventArgs args;
	args.button = button;
	args.clicks = 1;
	args.delta = 0;
	args.x = event->x();
	args.y = event->y();
	m_game_player->OnMouseDown(args.button, args.clicks, args.delta, args.x, args.y);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		x_down = event->x();
		y_down = event->y();
		press_timer.start();
	}
}

void PlayerWindow::OnMouseUp(QMouseEvent* event)
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

	MouseEventArgs args;
	args.button = button;
	args.clicks = 0;
	args.delta = 0;
	args.x = event->x();
	args.y = event->y();
	m_game_player->OnMouseUp(args.button, args.clicks, args.delta, args.x, args.y);

	if (press_timer.isActive())
	{
		press_timer.stop();
	}
}

void PlayerWindow::OnMouseMove(QMouseEvent* event)
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

	MouseEventArgs args;
	args.button = button;
	args.clicks = 0;
	args.delta = 0;
	args.x = event->x();
	args.y = event->y();
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

void PlayerWindow::OnWheel(QWheelEvent* event)
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

void PlayerWindow::OnChar(int charCode)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->OnChar(charCode);
}

void PlayerWindow::OnControlKey(int code)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->OnControlKey(code);
}

void PlayerWindow::OnClearConsole()
{
	m_ui.consoleTextEdit->clear();
}

