#include "ReflectorTuner.h"
#include "JsonUtils.h"

ReflectorTuner::ReflectorTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.tuner_width, SIGNAL(valueChanged(double)), this, SLOT(tuner_size_ValueChanged()));
	connect(m_ui.tuner_height, SIGNAL(valueChanged(double)), this, SLOT(tuner_size_ValueChanged()));	

	QJsonObject att = jobj["attributes"].toObject();
	if (att.contains("size"))
	{
		QString size = att["size"].toString();
		auto values = size.split(",");
		m_ui.tuner_width->setValue(values[0].toDouble());
		m_ui.tuner_height->setValue(values[1].toDouble());
	}
	

	obj3d_tuner = new Object3DTuner(this, jobj);
	obj3d_tuner->m_ui.logo->setPixmap(QPixmap(QString(":/images/plane.png")));
	m_ui.stack->addWidget(obj3d_tuner);
	connect(obj3d_tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner3d_update(QJsonObject)));


	initialized = true;
}

ReflectorTuner::~ReflectorTuner()
{

}

void ReflectorTuner::tuner3d_update(QJsonObject tuning)
{
	jobj = obj3d_tuner->jobj;	
	emit update(tuning);
}


void ReflectorTuner::tuner_size_ValueChanged()
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["size"] = QString::number(m_ui.tuner_width->value()) + ", " + QString::number(m_ui.tuner_height->value());

	QJsonObject att = jobj["attributes"].toObject();
	att["size"] = tuning["size"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;

	emit update(tuning);
}

