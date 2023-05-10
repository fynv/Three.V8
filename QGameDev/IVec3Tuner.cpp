#include <QPalette>
#include <QColorDialog>
#include "IVec3Tuner.h"

IVec3Tuner::IVec3Tuner(QWidget* parent) : QWidget(parent)
{
	m_ui.setupUi(this);
	m_ui.tuner_x->setMinimum(-32767); m_ui.tuner_x->setMaximum(32767);
	m_ui.tuner_y->setMinimum(-32767); m_ui.tuner_y->setMaximum(32767);
	m_ui.tuner_z->setMinimum(-32767); m_ui.tuner_z->setMaximum(32767);
	connect(m_ui.tuner_x, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged()));
	connect(m_ui.tuner_y, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged()));
	connect(m_ui.tuner_z, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged()));

}

IVec3Tuner::~IVec3Tuner()
{

}

void IVec3Tuner::get_value(int& x, int& y, int& z)
{
	x = m_ui.tuner_x->value();
	y = m_ui.tuner_y->value();
	z = m_ui.tuner_z->value();
}

void IVec3Tuner::set_value(int x, int y, int z)
{
	m_ui.tuner_x->setValue(x);
	m_ui.tuner_y->setValue(y);
	m_ui.tuner_z->setValue(z);
}

