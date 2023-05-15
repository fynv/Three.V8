#pragma once

#include <QJsonObject>
#include "ui_BackgroundSceneTuner.h"
#include "Tuner.h"

class BackgroundSceneTuner : public Tuner
{
	Q_OBJECT
public:
	BackgroundSceneTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~BackgroundSceneTuner();

private slots:
	void text_editing_finished();
	void btn_browse_Click();
	void tuner_near_ValueChanged(double value);
	void tuner_far_ValueChanged(double value);

private:
	Ui_BackgroundSceneTuner m_ui;
	bool initialized = false;

	void update_scene();
};

