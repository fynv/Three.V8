#include <QPainter>
#include <QDateTime>
#include <QKeyEvent>
#include <GL/glew.h>
#include "GLControl.h"

#include <unordered_map>

GLControl::GLControl(QWidget* parent) : QOpenGLWidget(parent)
{		
	this->setFocusPolicy(Qt::WheelFocus);
	this->setAttribute(Qt::WA_InputMethodEnabled, true);
	m_timer.setSingleShot(true);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

GLControl::~GLControl()
{

}

void GLControl::SetFramerate(float fps)
{
	m_interval = (unsigned)(1000.0f / fps);
	m_time_scheduled = QDateTime::currentMSecsSinceEpoch();
	this->update();
}

void GLControl::GetPhysicalSize(int& width, int& height)
{
	HWND hwnd = (HWND)(this->winId());
	RECT rect;
	GetClientRect(hwnd, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

void GLControl::initializeGL()
{
	glewInit();
	emit OnInit();
}

void GLControl::paintGL()
{
	QPainter painter;
	painter.begin(this);
	painter.beginNativePainting();

	int width, height;
	GetPhysicalSize(width, height);

	emit OnPaint(width, height);

	painter.endNativePainting();
	painter.end();

	if (m_interval > 0)
	{
		unsigned interval = m_interval;
		uint64_t t = QDateTime::currentMSecsSinceEpoch();
		int delta = (int)(int64_t)(t - m_time_scheduled);
		if (delta > interval) interval = 0;
		else if (delta > 0) interval -= delta;
		m_time_scheduled = t + interval;

		if (interval > 0)
		{	
			m_timer.start(std::chrono::milliseconds(interval));
		}
		else
		{
			this->update();
		}

	}
}

void GLControl::mousePressEvent(QMouseEvent* event) 
{
	emit OnMouseDown(event);
}


void GLControl::mouseReleaseEvent(QMouseEvent* event)
{
	emit OnMouseUp(event);
}


void GLControl::mouseMoveEvent(QMouseEvent* event)
{
	emit OnMouseMove(event);
}

void GLControl::wheelEvent(QWheelEvent* event)
{
	emit OnWheel(event);
}

void GLControl::keyPressEvent(QKeyEvent* event)
{
	//int c = event->key();
	//printf("%d\n", c);

	static std::unordered_map<int, int> code_map;
	if (code_map.size() < 1)
	{
		code_map[0x01000003] = 8;
		code_map[0x01000016] = 33;
		code_map[0x01000017] = 34;
		code_map[0x01000011] = 35;
		code_map[0x01000010] = 36;
		code_map[0x01000012] = 37;
		code_map[0x01000013] = 38;
		code_map[0x01000014] = 39;
		code_map[0x01000015] = 40;
		code_map[0x01000006] = 45;
		code_map[0x01000007] = 46;
	}

	m_ctrl_key = false;
	int c = event->key();
	if (code_map.find(c) != code_map.end())
	{
		emit OnControlKey(code_map[c]);
		m_ctrl_key = true;
	}	
}

void GLControl::keyReleaseEvent(QKeyEvent* event)
{
	if (m_ctrl_key) return;
	QString text = event->text();
	if (text.length() > 0)
	{		
		for (int i = 0; i < text.length(); i++)
		{
			emit OnChar(text.at(i).unicode());
		}
	}

}

void GLControl::inputMethodEvent(QInputMethodEvent* event)
{
	QString text = event->commitString();
	if (text.length() > 0)
	{
		for (int i = 0; i < text.length(); i++)
		{
			emit OnChar(text.at(i).unicode());
		}
	}
}