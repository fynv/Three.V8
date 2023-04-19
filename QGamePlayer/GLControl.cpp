#include <QPainter>
#include <QDateTime>
#include <GL/glew.h>
#include "GLControl.h"


GLControl::GLControl(QWidget* parent)
{		
	auto surfaceFormat = QSurfaceFormat::defaultFormat();
	surfaceFormat.setColorSpace(QSurfaceFormat::sRGBColorSpace);
	this->setFormat(surfaceFormat);
	this->setTextureFormat(GL_SRGB8_ALPHA8);
	this->setFocusPolicy(Qt::WheelFocus);
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

	emit OnPaint(this->width(), this->height());

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