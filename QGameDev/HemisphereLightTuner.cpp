#include <GamePlayer.h>
#include "HemisphereLightTuner.h"


HemisphereLightTuner::HemisphereLightTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.tuner_sky_color, SIGNAL(value_changed()), this, SLOT(tuner_sky_color_ValueChanged()));
	connect(m_ui.tuner_ground_color, SIGNAL(value_changed()), this, SLOT(tuner_ground_color_ValueChanged()));

	QJsonObject att = jobj["attributes"].toObject();

	{
		float r = 0.318f;
		float g = 0.318f;
		float b = 0.318f;
		if (att.contains("skyColor"))
		{
			auto color = att["skyColor"].toString().split(",");
			r = color[0].toFloat();
			g = color[1].toFloat();
			b = color[2].toFloat();
		}
		m_ui.tuner_sky_color->set_color(r, g, b);
	}

	{
		float r = 0.01f;
		float g = 0.025f;
		float b = 0.025f;
		if (att.contains("groundColor"))
		{
			auto color = att["groundColor"].toString().split(",");
			r = color[0].toFloat();
			g = color[1].toFloat();
			b = color[2].toFloat();
		}
		m_ui.tuner_ground_color->set_color(r, g, b);
	}
}

HemisphereLightTuner::~HemisphereLightTuner()
{

}

void HemisphereLightTuner::tuner_sky_color_ValueChanged()
{
	float r, g, b;
	m_ui.tuner_sky_color->get_color(r, g, b);

	QJsonObject tuning;
	tuning["skyColor"] = QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b);

	QJsonObject att = jobj["attributes"].toObject();
	att["skyColor"] = tuning["skyColor"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void HemisphereLightTuner::tuner_ground_color_ValueChanged()
{
	float r, g, b;
	m_ui.tuner_ground_color->get_color(r, g, b);

	QJsonObject tuning;
	tuning["groundColor"] = QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b);

	QJsonObject att = jobj["attributes"].toObject();
	att["groundColor"] = tuning["groundColor"];
	jobj["attributes"] = att;

	emit update(tuning);
}

