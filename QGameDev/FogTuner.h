#pragma once

#include <QJsonObject>
#include "ui_FogTuner.h"
#include "Tuner.h"

class FogTuner : public Tuner
{
	Q_OBJECT
public:
	FogTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~FogTuner();

private slots:
	void tuner_density_ValueChanged(double value);
	void tuner_color_ValueChanged();

private:
	Ui_FogTuner m_ui;	
};

