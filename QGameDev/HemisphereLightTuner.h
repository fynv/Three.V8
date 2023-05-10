#pragma once

#include <QJsonObject>
#include "ui_HemisphereLightTuner.h"
#include "Tuner.h"

class HemisphereLightTuner : public Tuner
{
	Q_OBJECT
public:
	HemisphereLightTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~HemisphereLightTuner();

private slots:
	void tuner_sky_color_ValueChanged();
	void tuner_ground_color_ValueChanged();

private:
	Ui_HemisphereLightTuner m_ui;
};

