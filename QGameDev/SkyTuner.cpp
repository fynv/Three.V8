#include <GamePlayer.h>
#include "SkyTuner.h"
#include "UniformSkyTuner.h"
#include "HemisphereSkyTuner.h"
#include "CubeSkyTuner.h"
#include "BackgroundSceneTuner.h"

SkyTuner::SkyTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.lst_types, SIGNAL(currentIndexChanged(int)), this, SLOT(lst_types_SelectionChanged(int)));

	QJsonObject att = jobj["attributes"].toObject();

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
	else if (type == "scene")
	{
		m_ui.lst_types->setCurrentIndex(3);
	}
	load_type(type);	
}

SkyTuner::~SkyTuner()
{

}

void SkyTuner::load_type(QString type)
{
	if (tuner != nullptr)
	{
		m_ui.property_area->removeWidget(tuner);
		tuner->deleteLater();
		tuner = nullptr;
	}

	if (type == "uniform")
	{
		tuner = new UniformSkyTuner(this, jobj);		
	}
	else if (type == "hemisphere")
	{
		tuner = new HemisphereSkyTuner(this, jobj);
	}
	else if (type == "cube")
	{
		tuner = new CubeSkyTuner(this, jobj);
	}
	else if (type == "scene")
	{
		tuner = new BackgroundSceneTuner(this, jobj);
	}

	if (tuner != nullptr)
	{
		m_ui.property_area->addWidget(tuner);
		connect(tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner_update(QJsonObject)));
	}
}

void SkyTuner::lst_types_SelectionChanged(int idx)
{	
	if (tuner == nullptr) return;
	if (idx < 0) return;
	static QString types[4] = { "uniform", "hemisphere", "cube", "scene" };

	QString type = types[idx];

	QJsonObject att;
	att["type"] = type;
	jobj["attributes"] = att;

	emit update(att);
	load_type(type);
}


void SkyTuner::tuner_update(QJsonObject tuning)
{
	jobj = tuner->jobj;
	emit update(tuning);
}
