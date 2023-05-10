#pragma once

#include <QJsonObject>
#include "ui_AmbientLightTuner.h"
#include "Tuner.h"

class AmbientLightTuner : public Tuner
{
	Q_OBJECT
public:
	AmbientLightTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~AmbientLightTuner();

private slots:
	void tuner_color_ValueChanged();

private:
	Ui_AmbientLightTuner m_ui;
};

