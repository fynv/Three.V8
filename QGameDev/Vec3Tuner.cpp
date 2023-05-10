#include <QPalette>
#include <QColorDialog>
#include "Vec3Tuner.h"

Vec3Tuner::Vec3Tuner(QWidget* parent) : QWidget(parent)
{
	m_ui.setupUi(this);
	m_ui.tuner_x->setMinimum(-INFINITY); m_ui.tuner_x->setMaximum(INFINITY);
	m_ui.tuner_y->setMinimum(-INFINITY); m_ui.tuner_y->setMaximum(INFINITY);
	m_ui.tuner_z->setMinimum(-INFINITY); m_ui.tuner_z->setMaximum(INFINITY);
	connect(m_ui.tuner_x, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged()));
	connect(m_ui.tuner_y, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged()));
	connect(m_ui.tuner_z, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged()));

}

Vec3Tuner::~Vec3Tuner()
{

}

void Vec3Tuner::get_value(float& x, float& y, float& z)
{
	x = (float)m_ui.tuner_x->value();
	y = (float)m_ui.tuner_y->value();
	z = (float)m_ui.tuner_z->value();
}

void Vec3Tuner::set_value(float x, float y, float z)
{	
	m_ui.tuner_x->setValue(x);
	m_ui.tuner_y->setValue(y);
	m_ui.tuner_z->setValue(z);
}

