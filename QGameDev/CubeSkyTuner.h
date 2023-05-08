#pragma once

#include <QJsonObject>
#include "ui_CubeSkyTuner.h"
#include "Tuner.h"

class CubeSkyTuner : public Tuner
{
	Q_OBJECT
public:
	CubeSkyTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~CubeSkyTuner();

private slots:
	void reload();
	void btn_browse_Click();

private:
	Ui_CubeSkyTuner m_ui;

	
};

