#pragma once

#include <QJsonObject>
#include "ui_SceneTuner.h"
#include "Tuner.h"

class SceneTuner : public Tuner
{
	Q_OBJECT
public:
	SceneTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~SceneTuner();	

private slots:	
	void btn_bake_Click();

signals:
	void generate(QJsonObject tuning);

private:
	Ui_SceneTuner m_ui;
	bool initialized = false;

};

