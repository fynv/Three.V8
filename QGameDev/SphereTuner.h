#pragma once

#include <QJsonObject>
#include "ui_SphereTuner.h"
#include "Tuner.h"
#include "Object3DTuner.h"
#include "SimpleMaterialTuner.h"

class SphereTuner : public Tuner
{
	Q_OBJECT
public:
	SphereTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~SphereTuner();

	Object3DTuner* obj3d_tuner = nullptr;
	SimpleMaterialTuner* material_tuner = nullptr;

private slots:
	void tuner3d_update(QJsonObject tuning);
	void tuner_material_update(QJsonObject tuning);	
	void tuner_radius_ValueChanged(double value);
	void tuner_width_segments_ValueChanged(int value);
	void tuner_height_segments_ValueChanged(int value);
	void chk_is_building_Checked(bool checked);

private:
	Ui_SphereTuner m_ui;
	bool initialized = false;

};

