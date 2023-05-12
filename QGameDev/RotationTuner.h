#pragma once

#include <QWidget>
#include "ui_RotationTuner.h"

class RotationTuner : public QWidget
{
	Q_OBJECT
public:
	RotationTuner(QWidget* parent);
	virtual ~RotationTuner();

	void get_value(int& x, int& y, int& z);
	void set_value(int x, int y, int z);

signals:
	void valueChanged();
	

private:
	Ui_RotationTuner m_ui;

};
