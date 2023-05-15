#include "SceneTuner.h"
#include "JsonUtils.h"


SceneTuner::SceneTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);

	m_ui.tuner_iterations->setValue(6);
	m_ui.tuner_num_rays->setMaximum(32768);
	m_ui.tuner_num_rays->setValue(2048);
	m_ui.tuner_num_rays->setSingleStep(512);
	connect(m_ui.btn_bake, SIGNAL(clicked()), this, SLOT(btn_bake_Click()));

	initialized = true;
}

SceneTuner::~SceneTuner()
{

}

void SceneTuner::btn_bake_Click()
{
	QJsonObject tuning;
	tuning["iterations"] = QString::number(m_ui.tuner_iterations->value());
	tuning["num_rays"] = QString::number(m_ui.tuner_num_rays->value());
	emit generate(tuning);

}
