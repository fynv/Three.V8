#pragma once

#include <QJsonObject>
#include "ui_LODProbeGridTuner.h"
#include "Tuner.h"

class LODProbeGridTuner : public Tuner
{
	Q_OBJECT
public:
	LODProbeGridTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~LODProbeGridTuner();

	void update_result(QJsonObject res) override;
	void initialize_result(QString res);

signals:
	void generate(QJsonObject tuning);
	void initialize();

private slots:
	void load_data();
	void btn_browse_Click();
	void tuner_divisions_ValueChanged();
	void tuner_coverage_min_ValueChanged();
	void tuner_coverage_max_ValueChanged();
	void tuner_sub_div_ValueChanged(int value);
	void tuner_normal_bias_ValueChanged(double value);
	void btn_start_Click();
	void btn_auto_detect_Click();
	void chk_per_primitive_Checked(bool checked);


private:
	Ui_LODProbeGridTuner m_ui;
	bool initialized = false;

};

