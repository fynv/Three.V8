#pragma once

#include <QJsonObject>
#include "ui_BoxTuner.h"
#include "Tuner.h"
#include "Object3DTuner.h"
#include "SimpleMaterialTuner.h"

class BoxTuner : public Tuner
{
	Q_OBJECT
public:
	BoxTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~BoxTuner();

	Object3DTuner* obj3d_tuner = nullptr;
	SimpleMaterialTuner* material_tuner = nullptr;

private slots:
	void tuner3d_update(QJsonObject tuning);
	void tuner_material_update(QJsonObject tuning);
	void tuner_size_ValueChanged();
	void chk_is_building_Checked(bool checked);

private:
	Ui_BoxTuner m_ui;
	bool initialized = false;

};

