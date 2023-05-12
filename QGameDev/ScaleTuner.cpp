#include "ScaleTuner.h"

ScaleTuner::ScaleTuner(QWidget* parent) : QWidget(parent)
{
	m_ui.setupUi(this);	
	m_ui.text_x->installEventFilter(this);
	m_ui.text_y->installEventFilter(this);
	m_ui.text_z->installEventFilter(this);
	connect(m_ui.text_x, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged()));
	connect(m_ui.text_y, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged()));
	connect(m_ui.text_z, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged()));
	connect(m_ui.btn_incr, SIGNAL(clicked()), this, SLOT(btn_incr_Click()));
	connect(m_ui.btn_decr, SIGNAL(clicked()), this, SLOT(btn_decr_Click()));

}

ScaleTuner::~ScaleTuner()
{

}

void ScaleTuner::get_value(float& x, float& y, float& z)
{
	x = (float)m_ui.text_x->value();
	y = (float)m_ui.text_y->value();
	z = (float)m_ui.text_z->value();
}

void ScaleTuner::set_value(float x, float y, float z)
{
	m_ui.text_x->setValue(x);
	m_ui.text_y->setValue(y);
	m_ui.text_z->setValue(z);
}


bool ScaleTuner::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Wheel && 
		(obj == m_ui.text_x || obj == m_ui.text_y || obj == m_ui.text_z))
	{
		return true;
	}
	return false;
}

void ScaleTuner::btn_decr_Click()
{
	float x, y, z;
	get_value(x, y, z);
	x /= step;
	y /= step;
	z /= step;
	set_value(x, y, z);
	emit valueChanged();
}

void ScaleTuner::btn_incr_Click()
{
	float x, y, z;
	get_value(x, y, z);
	x *= step;
	y *= step;
	z *= step;
	set_value(x, y, z);
	emit valueChanged();

}