#include <GamePlayer.h>
#include "AmbientLightTuner.h"


AmbientLightTuner::AmbientLightTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.tuner_color, SIGNAL(valueChanged()), this, SLOT(tuner_color_ValueChanged()));

	QJsonObject att = jobj["attributes"].toObject();

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

AmbientLightTuner::~AmbientLightTuner()
{

}

void AmbientLightTuner::tuner_color_ValueChanged()
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