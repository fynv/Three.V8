#pragma once

#include <QWidget>
#include "ui_ColorTuner.h"

class ColorTuner : public QWidget
{
	Q_OBJECT
public:
	ColorTuner(QWidget* parent);
	virtual ~ColorTuner();

	void get_color(float& r, float& g, float& b);
	void set_color(float r, float g, float b);

signals:
	void value_changed();

private:
	Ui_ColorTuner m_ui;
	float r, g, b;
	void update();

private slots:
	void btn_Click();
};
