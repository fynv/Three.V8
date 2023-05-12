#pragma once

#include <QJsonObject>
#include "ui_GroupTuner.h"
#include "Tuner.h"
#include "Object3DTuner.h"

class GroupTuner : public Tuner
{
	Q_OBJECT
public:
	GroupTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~GroupTuner();

	Object3DTuner* obj3d_tuner = nullptr;

private slots:
	void tuner_update(QJsonObject tuning);

private:
	Ui_GroupTuner m_ui;

};

