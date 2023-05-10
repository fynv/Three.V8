#pragma once

#include <QJsonObject>
#include "ui_ProbeGridTuner.h"
#include "Tuner.h"

class ProbeGridTuner : public Tuner
{
	Q_OBJECT
public:
	ProbeGridTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~ProbeGridTuner();

	void update_result(QJsonObject res) override;

signals:
	void generate(QJsonObject tuning);

private slots:
	void load_data();
	void btn_browse_Click();
	void tuner_divisions_ValueChanged();
	void tuner_coverage_min_ValueChanged();
	void tuner_coverage_max_ValueChanged();
	void tuner_ypower_ValueChanged(double value);
	void tuner_normal_bias_ValueChanged(double value);
	void btn_start_Click();
	void btn_auto_detect_Click();
	void chk_per_primitive_Checked(bool checked);

private:
	Ui_ProbeGridTuner m_ui;
	bool initialized = false;

};

