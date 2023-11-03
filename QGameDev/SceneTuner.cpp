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

	m_ui.text_path_heightmap->setText("assets/height_map.bin");
	m_ui.text_width->setText("2048");
	m_ui.text_height->setText("2048");

	m_ui.text_path_bvh->setText("assets/bvh.bin");
	
	connect(m_ui.btnGenerate, SIGNAL(clicked()), this, SLOT(btn_generate_Click()));
	connect(m_ui.btn_bake, SIGNAL(clicked()), this, SLOT(btn_bake_Click()));
	connect(m_ui.btnSave, SIGNAL(clicked()), this, SLOT(btn_save_Click()));

	initialized = true;
}

SceneTuner::~SceneTuner()
{

}

void SceneTuner::btn_bake_Click()
{
	QJsonObject tuning;
	tuning["type"] = QString("lightmap");
	tuning["iterations"] = QString::number(m_ui.tuner_iterations->value());
	tuning["num_rays"] = QString::number(m_ui.tuner_num_rays->value());
	emit generate(tuning);
}


void SceneTuner::btn_generate_Click()
{
	QJsonObject tuning;
	tuning["type"] = QString("heightmap");
	tuning["path"] = m_ui.text_path_heightmap->text();
	tuning["width"] = m_ui.text_width->text();
	tuning["height"] = m_ui.text_height->text();
	emit generate(tuning);
}

void SceneTuner::btn_save_Click()
{
	QJsonObject tuning;
	tuning["type"] = QString("bvh");
	tuning["path"] = m_ui.text_path_bvh->text();
	emit generate(tuning);
}