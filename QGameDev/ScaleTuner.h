#pragma once

#include <QWidget>
#include "ui_ScaleTuner.h"

class ScaleTuner : public QWidget
{
	Q_OBJECT
public:
	ScaleTuner(QWidget* parent);
	virtual ~ScaleTuner();

	float step = 1.1f;

	void get_value(float& x, float& y, float& z);
	void set_value(float x, float y, float z);


protected:
	bool eventFilter(QObject* obj, QEvent* event) override;

signals:
	void valueChanged();

private slots:
	void btn_decr_Click();
	void btn_incr_Click();


private:
	Ui_ScaleTuner m_ui;

};
