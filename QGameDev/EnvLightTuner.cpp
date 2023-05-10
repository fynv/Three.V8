#include <GamePlayer.h>
#include "EnvLightTuner.h"
#include "AmbientLightTuner.h"
#include "HemisphereLightTuner.h"
#include "EnvMapTuner.h"
#include "ProbeGridTuner.h"
#include "LODProbeGridTuner.h"
#include "JsonUtils.h"

EnvLightTuner::EnvLightTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.lst_types, SIGNAL(currentIndexChanged(int)), this, SLOT(lst_types_SelectionChanged(int)));
	connect(m_ui.chk_dynamic, SIGNAL(toggled(bool)), this, SLOT(chk_dynamic_Checked(bool)));

	QJsonObject att = jobj["attributes"].toObject();

	if (att.contains("dynamic_map"))
	{
		bool dynamic_map = decodeJsonBoolLiteral(att["dynamic_map"].toString());
		m_ui.chk_dynamic->setChecked(dynamic_map);
	}

	QString type = "hemisphere";
	if (att.contains("type"))
	{
		type = att["type"].toString();
	}

	if (type == "uniform")
	{
		m_ui.lst_types->setCurrentIndex(0);
	}
	else if (type == "hemisphere")
	{
		m_ui.lst_types->setCurrentIndex(1);
	}
	else if (type == "cube")
	{
		m_ui.lst_types->setCurrentIndex(2);
	}
	else if (type == "probe_grid")
	{
		m_ui.lst_types->setCurrentIndex(3);
	}
	else if (type == "lod_probe_grid")
	{
		m_ui.lst_types->setCurrentIndex(4);
	}
	load_type(type);
}

EnvLightTuner::~EnvLightTuner()
{
	
}

void EnvLightTuner::update_result(QJsonObject res)
{
	tuner->update_result(res);
	jobj = tuner->jobj;
}

void EnvLightTuner::initialize_result(QString res)
{
	LODProbeGridTuner* lod_tuner = dynamic_cast<LODProbeGridTuner*>(tuner);
	if (lod_tuner != nullptr)
	{
		lod_tuner->initialize_result(res);
	}
}

void EnvLightTuner::load_type(QString type)
{
	if (tuner != nullptr)
	{
		m_ui.property_area->removeWidget(tuner);
		tuner->deleteLater();
		tuner = nullptr;
	}

	if (type == "uniform")
	{
		tuner = new AmbientLightTuner(this, jobj);
	}
	else if (type == "hemisphere")
	{
		tuner = new HemisphereLightTuner(this, jobj);
	}
	else if (type == "cube")
	{
		tuner = new EnvMapTuner(this, jobj);
		connect(tuner, SIGNAL(generate(QJsonObject)), this, SLOT(tuner_generate(QJsonObject)));
	}
	else if (type == "probe_grid")
	{
		tuner = new ProbeGridTuner(this, jobj);
		connect(tuner, SIGNAL(generate(QJsonObject)), this, SLOT(tuner_generate(QJsonObject)));
	}
	else if (type == "lod_probe_grid")
	{
		tuner = new LODProbeGridTuner(this, jobj);
		connect(tuner, SIGNAL(generate(QJsonObject)), this, SLOT(tuner_generate(QJsonObject)));
		connect(tuner, SIGNAL(initialize()), this, SIGNAL(initialize()));
	}

	if (tuner != nullptr)
	{
		m_ui.property_area->addWidget(tuner);
		connect(tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner_update(QJsonObject)));
	}
}

void EnvLightTuner::lst_types_SelectionChanged(int idx)
{
	if (tuner == nullptr) return;
	if (idx < 0) return;
	static QString types[5] = { "uniform", "hemisphere", "cube", "probe_grid", "lod_probe_grid" };

	QString type = types[idx];

	QJsonObject att;
	att["type"] = type;
	jobj["attributes"] = att;

	emit update(att);
	load_type(type);
}

void EnvLightTuner::chk_dynamic_Checked(bool checked)
{
	if (tuner == nullptr) return;

	QJsonObject tuning;
	tuning["dynamic_map"] = encodeJsonBoolLiteral(checked);

	QJsonObject att = jobj["attributes"].toObject();
	att["dynamic_map"] = tuning["dynamic_map"];

	jobj["attributes"] = att;
	tuner->jobj = jobj;

	emit update(tuning);
}

void EnvLightTuner::tuner_update(QJsonObject tuning)
{
	jobj = tuner->jobj;
	emit update(tuning);
}

void EnvLightTuner::tuner_generate(QJsonObject tuning)
{
	jobj = tuner->jobj;
	emit generate(tuning);
}

