#pragma once

#include <QJsonObject>
#include "ui_SkyTuner.h"
#include "Tuner.h"

class SkyTuner : public Tuner
{
	Q_OBJECT
public:
	SkyTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~SkyTuner();

private slots:
	void lst_types_SelectionChanged(int idx);
	void tuner_update(QJsonObject tuning);

private:
	Ui_SkyTuner m_ui;
	Tuner* tuner = nullptr;

	void load_type(QString type);
};

