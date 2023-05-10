#pragma once

#include <QWidget>
#include "ui_Vec3Tuner.h"

class Vec3Tuner : public QWidget
{
	Q_OBJECT
public:
	Vec3Tuner(QWidget* parent);
	virtual ~Vec3Tuner();

	Ui_Vec3Tuner m_ui;

	void get_value(float& x, float& y, float& z);
	void set_value(float x, float y, float z);

signals:
	void valueChanged();
};
