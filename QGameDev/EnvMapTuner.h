#pragma once

#include <QJsonObject>
#include "ui_EnvMapTuner.h"
#include "Tuner.h"

class EnvMapTuner : public Tuner
{
	Q_OBJECT
public:
	EnvMapTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~EnvMapTuner();

signals:
	void generate(QJsonObject tuning);

private slots:
	void tuner_probe_pos_ValueChanged();
	void reload();
	void btn_generate_Click();
	void btn_browse_Click();
	void load_sh();
	void btn_browse_sh_Click();
	void chk_irr_only_Checked(bool checked);

private:
	Ui_EnvMapTuner m_ui;
	bool initialized = false;

};

