#pragma once

#include <QWidget>
#include "ui_AngleTuner.h"

class AngleTuner : public QWidget
{
	Q_OBJECT
public:
	AngleTuner(QWidget* parent);
	virtual ~AngleTuner();

	int get_value();
	void set_value(int value);

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;

signals:
	void valueChanged(int value);

private:
	Ui_AngleTuner m_ui;

private slots:
	void update_value(int value);


};
