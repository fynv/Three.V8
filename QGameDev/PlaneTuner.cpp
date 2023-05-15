#include "PlaneTuner.h"
#include "JsonUtils.h"

PlaneTuner::PlaneTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);	
	connect(m_ui.tuner_width, SIGNAL(valueChanged(double)), this, SLOT(tuner_size_ValueChanged()));
	connect(m_ui.tuner_height, SIGNAL(valueChanged(double)), this, SLOT(tuner_size_ValueChanged()));
	connect(m_ui.chk_is_building, SIGNAL(toggled(bool)), this, SLOT(chk_is_building_Checked(bool)));

	QJsonObject att = jobj["attributes"].toObject();
	if (att.contains("size"))
	{
		QString size = att["size"].toString();
		auto values = size.split(",");
		m_ui.tuner_width->setValue(values[0].toDouble());
		m_ui.tuner_height->setValue(values[1].toDouble());
	}

	if (att.contains("is_building"))
	{
		bool is_building = decodeJsonBoolLiteral(att["is_building"].toString());
		m_ui.chk_is_building->setChecked(is_building);
	}

	obj3d_tuner = new Object3DTuner(this, jobj);
	obj3d_tuner->m_ui.logo->setPixmap(QPixmap(QString(":/images/plane.png")));
	m_ui.stack->addWidget(obj3d_tuner);
	connect(obj3d_tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner3d_update(QJsonObject)));


	material_tuner = new SimpleMaterialTuner(this, jobj);
	m_ui.stack2->addWidget(material_tuner);
	connect(material_tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner_material_update(QJsonObject)));


	initialized = true;
}

PlaneTuner::~PlaneTuner()
{

}

void PlaneTuner::tuner3d_update(QJsonObject tuning)
{
	jobj = obj3d_tuner->jobj;
	material_tuner->jobj = jobj;
	emit update(tuning);
}

void PlaneTuner::tuner_material_update(QJsonObject tuning)
{
	jobj = material_tuner->jobj;
	obj3d_tuner->jobj = jobj;
	emit update(tuning);
}

void PlaneTuner::tuner_size_ValueChanged()
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["size"] = QString::number(m_ui.tuner_width->value()) + ", " + QString::number(m_ui.tuner_height->value());

	QJsonObject att = jobj["attributes"].toObject();
	att["size"] = tuning["size"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;
	material_tuner->jobj = jobj;

	emit update(tuning);
}


void PlaneTuner::chk_is_building_Checked(bool checked)
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

