#pragma once

#include <QOpenGLWidget>
#include <QTimer>

class GLControl : public QOpenGLWidget
{
	Q_OBJECT
public:
	GLControl(QWidget* parent);
	virtual ~GLControl();

	void SetFramerate(float fps);

signals:
	void OnInit();
	void OnPaint(int width, int height);

	void OnMouseDown(QMouseEvent* event);
	void OnMouseUp(QMouseEvent* event);
	void OnMouseMove(QMouseEvent* event);
	void OnWheel(QWheelEvent* event);

protected:
	void initializeGL() override;
	void paintGL() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
	unsigned m_interval = 0;
	uint64_t m_time_scheduled;
	QTimer m_timer;

};

