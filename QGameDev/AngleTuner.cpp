#include "AngleTuner.h"


AngleTuner::AngleTuner(QWidget* parent) : QWidget(parent)
{
	m_ui.setupUi(this);
	connect(m_ui.text, SIGNAL(valueChanged(int)), this, SLOT(update_value(int)));
	connect(m_ui.slider, SIGNAL(valueChanged(int)), this, SLOT(update_value(int)));
	m_ui.text->installEventFilter(this);
}

AngleTuner::~AngleTuner()
{

}

int AngleTuner::get_value()
{
	return m_ui.text->value();
}

void AngleTuner::set_value(int value)
{
	if (value != m_ui.text->value())
	{
		m_ui.text->setValue(value);
		m_ui.slider->setValue(value);
	}
}

bool AngleTuner::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Wheel && obj == m_ui.text)
	{		
		return true;
	}
	return false;
}

void AngleTuner::update_value(int value)
{
	while (value > 180) value -= 360;
	while (value < -180) value += 360;

	m_ui.text->setValue(value);
	m_ui.slider->setValue(value);
	emit valueChanged(value);
}


