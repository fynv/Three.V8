#pragma once

#include <QJsonObject>
#include "ui_HemisphereSkyTuner.h"
#include "Tuner.h"

class HemisphereSkyTuner : public Tuner
{
	Q_OBJECT
public:
	HemisphereSkyTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~HemisphereSkyTuner();

private slots:
	void tuner_sky_color_ValueChanged();
	void tuner_ground_color_ValueChanged();

private:
	Ui_HemisphereSkyTuner m_ui;
};

