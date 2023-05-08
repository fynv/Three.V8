#pragma once

#include <QJsonObject>
#include "ui_UniformSkyTuner.h"
#include "Tuner.h"

class UniformSkyTuner : public Tuner
{
	Q_OBJECT
public:
	UniformSkyTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~UniformSkyTuner();

private slots:
	void tuner_color_ValueChanged();

private:
	Ui_UniformSkyTuner m_ui;
};

