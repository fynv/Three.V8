#pragma once

#include <QJsonObject>
#include "ui_SimpleMaterialTuner.h"
#include "Tuner.h"

class SimpleMaterialTuner : public Tuner
{
	Q_OBJECT
public:
	SimpleMaterialTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~SimpleMaterialTuner();

private slots:
	void tuner_color_ValueChanged();
	void load_texture();
	void btn_browse_Click();
	void tuner_metalness_ValueChanged(double value);
	void tuner_roughness_ValueChanged(double value);

private:
	Ui_SimpleMaterialTuner m_ui;

};

