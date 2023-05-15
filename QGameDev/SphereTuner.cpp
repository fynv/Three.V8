#include "SphereTuner.h"
#include "JsonUtils.h"

SphereTuner::SphereTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.tuner_radius, SIGNAL(valueChanged(double)), this, SLOT(tuner_radius_ValueChanged(double)));
	connect(m_ui.tuner_width_segments, SIGNAL(valueChanged(int)), this, SLOT(tuner_width_segments_ValueChanged(int)));
	connect(m_ui.tuner_height_segments, SIGNAL(valueChanged(int)), this, SLOT(tuner_height_segments_ValueChanged(int)));
	connect(m_ui.chk_is_building, SIGNAL(toggled(bool)), this, SLOT(chk_is_building_Checked(bool)));

	QJsonObject att = jobj["attributes"].toObject();

	if (att.contains("radius"))
	{
		m_ui.tuner_radius->setValue(att["radius"].toString().toDouble());
	}

	if (att.contains("widthSegments"))
	{
		m_ui.tuner_width_segments->setValue(att["widthSegments"].toString().toInt());
	}

	if (att.contains("heightSegments"))
	{
		m_ui.tuner_height_segments->setValue(att["heightSegments"].toString().toInt());
	}

	if (att.contains("is_building"))
	{
		bool is_building = decodeJsonBoolLiteral(att["is_building"].toString());
		m_ui.chk_is_building->setChecked(is_building);
	}

	obj3d_tuner = new Object3DTuner(this, jobj);
	obj3d_tuner->m_ui.logo->setPixmap(QPixmap(QString(":/images/sphere.png")));
	m_ui.stack->addWidget(obj3d_tuner);
	connect(obj3d_tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner3d_update(QJsonObject)));

	material_tuner = new SimpleMaterialTuner(this, jobj);
	m_ui.stack2->addWidget(material_tuner);
	connect(material_tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner_material_update(QJsonObject)));

	initialized = true;
}

SphereTuner::~SphereTuner()
{

}

void SphereTuner::tuner3d_update(QJsonObject tuning)
{
	jobj = obj3d_tuner->jobj;
	material_tuner->jobj = jobj;
	emit update(tuning);
}

void SphereTuner::tuner_material_update(QJsonObject tuning)
{
	jobj = material_tuner->jobj;
	obj3d_tuner->jobj = jobj;
	emit update(tuning);
}

void SphereTuner::tuner_radius_ValueChanged(double value)
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["radius"] = QString::number(value);

	QJsonObject att = jobj["attributes"].toObject();
	att["radius"] = tuning["radius"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;
	material_tuner->jobj = jobj;

	emit update(tuning);
}

void SphereTuner::tuner_width_segments_ValueChanged(int value)
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["widthSegments"] = QString::number(value);

	QJsonObject att = jobj["attributes"].toObject();
	att["widthSegments"] = tuning["widthSegments"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;
	material_tuner->jobj = jobj;

	emit update(tuning);
}

void SphereTuner::tuner_height_segments_ValueChanged(int value)
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["heightSegments"] = QString::number(value);

	QJsonObject att = jobj["attributes"].toObject();
	att["heightSegments"] = tuning["heightSegments"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;
	material_tuner->jobj = jobj;

	emit update(tuning);
}

void SphereTuner::chk_is_building_Checked(bool checked)
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["is_building"] = encodeJsonBoolLiteral(checked);

	QJsonObject att = jobj["attributes"].toObject();
	att["is_building"] = tuning["is_building"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;
	material_tuner->jobj = jobj;

	emit update(tuning);
}

