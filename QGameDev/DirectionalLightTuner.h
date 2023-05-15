#pragma once

#include <QJsonObject>
#include "ui_DirectionalLightTuner.h"
#include "Tuner.h"
#include "Object3DTuner.h"


class DirectionalLightTuner : public Tuner
{
	Q_OBJECT
public:
	DirectionalLightTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~DirectionalLightTuner();

	Object3DTuner* obj3d_tuner = nullptr;

	void update_result(QJsonObject ret) override;

private slots:
	void tuner3d_update(QJsonObject tuning);
	void tuner_intensity_ValueChanged(double value);
	void tuner_color_ValueChanged();
	void update_target();
	void update_shadow();
	void chk_cast_shadow_Checked(bool checked);
	void tuner_area_ValueChanged();
	void tuner_radius_ValueChanged(double value);
	void tuner_bias_ValueChanged(double value);
	void btn_auto_detect_Click();
	void chk_force_cull_Checked(bool checked);
	
private:
	Ui_DirectionalLightTuner m_ui;
	bool initialized = false;

};

