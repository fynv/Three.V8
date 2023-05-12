#pragma once

#include <QJsonObject>
#include "ui_Object3DTuner.h"
#include "Tuner.h"

class Object3DTuner : public Tuner
{
	Q_OBJECT
public:
	Ui_Object3DTuner m_ui;

	Object3DTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~Object3DTuner();

private slots:
	void update_name();
	void tuner_pos_ValueChanged();
	void tuner_rot_ValueChanged();
	void tuner_scale_ValueChanged();
	

};

