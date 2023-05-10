#pragma once

#include <QJsonObject>
#include "ui_EnvLightTuner.h"
#include "Tuner.h"

class EnvLightTuner : public Tuner
{
	Q_OBJECT
public:
	EnvLightTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~EnvLightTuner();

	void update_result(QJsonObject res) override;
	void initialize_result(QString res);

signals:
	void generate(QJsonObject tuning);
	void initialize();

private slots:
	void lst_types_SelectionChanged(int idx);
	void chk_dynamic_Checked(bool checked);
	void tuner_update(QJsonObject tuning);
	void tuner_generate(QJsonObject tuning);

private:
	Ui_EnvLightTuner m_ui;
	Tuner* tuner = nullptr;

	void load_type(QString type);
};

