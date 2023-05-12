#include <GamePlayer.h>
#include "FogTuner.h"

FogTuner::FogTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.tuner_density, SIGNAL(valueChanged(double)), this, SLOT(tuner_density_ValueChanged(double)));
	connect(m_ui.tuner_color, SIGNAL(valueChanged()), this, SLOT(tuner_color_ValueChanged()));

	QJsonObject att = jobj["attributes"].toObject();

	m_ui.tuner_density->setValue(0.1f);

	if (att.contains("density"))
	{
		m_ui.tuner_density->setValue(att["density"].toString().toDouble());
	}

	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	if (att.contains("color"))
	{
		auto color = att["color"].toString().split(",");
		r = color[0].toFloat();
		g = color[1].toFloat();
		b = color[2].toFloat();
	}
	m_ui.tuner_color->set_color(r, g, b);	
}

FogTuner::~FogTuner()
{

}

void FogTuner::tuner_density_ValueChanged(double value)
{
	QJsonObject tuning;
	tuning["density"] = QString::number(m_ui.tuner_density->value());

	QJsonObject att = jobj["attributes"].toObject();
	att["density"] = tuning["density"];
	jobj["attributes"] = att;

	emit update(tuning);
}


void FogTuner::tuner_color_ValueChanged()
{
	float r, g, b;
	m_ui.tuner_color->get_color(r, g, b);

	QJsonObject tuning;
	tuning["color"] = QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b);

	QJsonObject att = jobj["attributes"].toObject();
	att["color"] = tuning["color"];
	jobj["attributes"] = att;

	emit update(tuning);
}