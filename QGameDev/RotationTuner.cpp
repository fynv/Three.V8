#include "RotationTuner.h"


RotationTuner::RotationTuner(QWidget* parent) : QWidget(parent)
{
	m_ui.setupUi(this);
	connect(m_ui.tuner_x, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged()));
	connect(m_ui.tuner_y, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged()));
	connect(m_ui.tuner_z, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged()));
}

RotationTuner::~RotationTuner()
{

}

void RotationTuner::get_value(int& x, int& y, int& z)
{
	x = m_ui.tuner_x->get_value();
	y = m_ui.tuner_y->get_value();
	z = m_ui.tuner_z->get_value();
}

void RotationTuner::set_value(int x, int y, int z)
{
	m_ui.tuner_x->set_value(x);
	m_ui.tuner_y->set_value(y);
	m_ui.tuner_z->set_value(z);
}



