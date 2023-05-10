#pragma once

#include <QWidget>
#include "ui_IVec3Tuner.h"

class IVec3Tuner : public QWidget
{
	Q_OBJECT
public:
	IVec3Tuner(QWidget* parent);
	virtual ~IVec3Tuner();

	Ui_IVec3Tuner m_ui;

	void get_value(int& x, int& y, int& z);
	void set_value(int x, int y, int z);

signals:
	void valueChanged();
};
