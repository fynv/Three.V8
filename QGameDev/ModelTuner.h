#pragma once

#include <QJsonObject>
#include "ui_ModelTuner.h"
#include "Tuner.h"
#include "Object3DTuner.h"

class ModelTuner : public Tuner
{
	Q_OBJECT
public:
	ModelTuner(QWidget* parent, const QJsonObject& jobj, bool is_avatar);
	virtual ~ModelTuner();

	Object3DTuner* obj3d_tuner = nullptr;	

private slots:
	void tuner3d_update(QJsonObject tuning);	
	void load_model();
	void btn_browse_Click();
	void chk_is_building_Checked(bool checked);
	void set_lightmap();
	void btn_browse_lightmap_Click();

private:
	Ui_ModelTuner m_ui;
	bool initialized = false;

};

