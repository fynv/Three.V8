#include "Object3DTuner.h"

Object3DTuner::Object3DTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.text_name, SIGNAL(editingFinished()), this, SLOT(update_name()));
	connect(m_ui.tuner_pos, SIGNAL(valueChanged()), this, SLOT(tuner_pos_ValueChanged()));
	connect(m_ui.tuner_rot, SIGNAL(valueChanged()), this, SLOT(tuner_rot_ValueChanged()));
	connect(m_ui.tuner_scale, SIGNAL(valueChanged()), this, SLOT(tuner_scale_ValueChanged()));

	m_ui.tuner_pos->m_ui.tuner_x->setSingleStep(0.5f);
	m_ui.tuner_pos->m_ui.tuner_y->setSingleStep(0.5f);
	m_ui.tuner_pos->m_ui.tuner_z->setSingleStep(0.5f);	

	QJsonObject att = jobj["attributes"].toObject();
	if (att.contains("name"))
	{
		m_ui.text_name->setText(att["name"].toString());
	}
	else
	{
		m_ui.text_name->setText(jobj["tagName"].toString());
	}

	if (att.contains("position"))
	{
		QString position = att["position"].toString();
		auto values = position.split(",");
		float x = values[0].toFloat();
		float y = values[1].toFloat();
		float z = values[2].toFloat();
		m_ui.tuner_pos->set_value(x, y, z);
	}

	if (att.contains("rotation"))
	{
		QString rotation = att["rotation"].toString();
		auto values = rotation.split(",");
		float x = values[0].toFloat();
		float y = values[1].toFloat();
		float z = values[2].toFloat();
		m_ui.tuner_rot->set_value(x, y, z);
	}

	if (att.contains("scale"))
	{
		QString scale = att["scale"].toString();
		auto values = scale.split(",");
		float x = values[0].toFloat();
		float y = values[1].toFloat();
		float z = values[2].toFloat();
		m_ui.tuner_scale->set_value(x, y, z);
	}

	initialized = true;
}

Object3DTuner::~Object3DTuner()
{

}

void Object3DTuner::update_name()
{
	QJsonObject tuning;
	tuning["name"] = m_ui.text_name->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["name"] = tuning["name"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void Object3DTuner::tuner_pos_ValueChanged()
{
	if (!initialized) return;

	float x, y, z;
	m_ui.tuner_pos->get_value(x, y, z);
	QJsonObject tuning;
	tuning["position"] = QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z);

	QJsonObject att = jobj["attributes"].toObject();
	att["position"] = tuning["position"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void Object3DTuner::tuner_rot_ValueChanged()
{
	if (!initialized) return;

	int x, y, z;
	m_ui.tuner_rot->get_value(x, y, z);
	QJsonObject tuning;
	tuning["rotation"] = QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z);

	QJsonObject att = jobj["attributes"].toObject();
	att["rotation"] = tuning["rotation"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void Object3DTuner::tuner_scale_ValueChanged()
{
	if (!initialized) return;

	float x, y, z;
	m_ui.tuner_scale->get_value(x, y, z);
	QJsonObject tuning;
	tuning["scale"] = QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z);

	QJsonObject att = jobj["attributes"].toObject();
	att["scale"] = tuning["scale"];
	jobj["attributes"] = att;

	emit update(tuning);
}

