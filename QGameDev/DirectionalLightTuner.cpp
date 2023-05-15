#include "DirectionalLightTuner.h"
#include "JsonUtils.h"


DirectionalLightTuner::DirectionalLightTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	m_ui.tuner_intensity->setMinimum(-INFINITY);
	m_ui.tuner_intensity->setMaximum(INFINITY);
	m_ui.tuner_intensity->setValue(1.0f);
	m_ui.tuner_width->setValue(512);
	m_ui.tuner_height->setValue(512);
	m_ui.tuner_left->setMinimum(-INFINITY);
	m_ui.tuner_left->setMaximum(INFINITY);
	m_ui.tuner_left->setValue(-1.0f);
	m_ui.tuner_right->setMinimum(-INFINITY);
	m_ui.tuner_right->setMaximum(INFINITY);
	m_ui.tuner_right->setValue(1.0f);
	m_ui.tuner_bottom->setMinimum(-INFINITY);
	m_ui.tuner_bottom->setMaximum(INFINITY);
	m_ui.tuner_bottom->setValue(-1.0f);
	m_ui.tuner_top->setMinimum(-INFINITY);
	m_ui.tuner_top->setMaximum(INFINITY);
	m_ui.tuner_top->setValue(1.0f);
	m_ui.tuner_near->setMinimum(-INFINITY);
	m_ui.tuner_near->setMaximum(INFINITY);
	m_ui.tuner_near->setValue(0.0f);
	m_ui.tuner_far->setMinimum(-INFINITY);
	m_ui.tuner_far->setMaximum(INFINITY);
	m_ui.tuner_far->setValue(10.0f);

	connect(m_ui.tuner_intensity, SIGNAL(valueChanged(double)), this, SLOT(tuner_intensity_ValueChanged(double)));
	connect(m_ui.tuner_color, SIGNAL(valueChanged()), this, SLOT(tuner_color_ValueChanged()));
	connect(m_ui.text_target, SIGNAL(editingFinished()), this, SLOT(update_target()));
	connect(m_ui.chk_cast_shadow, SIGNAL(toggled(bool)), this, SLOT(chk_cast_shadow_Checked(bool)));
	connect(m_ui.tuner_width, SIGNAL(valueChanged(int)), this, SLOT(update_shadow()));
	connect(m_ui.tuner_height, SIGNAL(valueChanged(int)), this, SLOT(update_shadow()));
	connect(m_ui.tuner_left, SIGNAL(valueChanged(double)), this, SLOT(tuner_area_ValueChanged()));
	connect(m_ui.tuner_right, SIGNAL(valueChanged(double)), this, SLOT(tuner_area_ValueChanged()));
	connect(m_ui.tuner_bottom, SIGNAL(valueChanged(double)), this, SLOT(tuner_area_ValueChanged()));
	connect(m_ui.tuner_top, SIGNAL(valueChanged(double)), this, SLOT(tuner_area_ValueChanged()));
	connect(m_ui.tuner_near, SIGNAL(valueChanged(double)), this, SLOT(tuner_area_ValueChanged()));
	connect(m_ui.tuner_far, SIGNAL(valueChanged(double)), this, SLOT(tuner_area_ValueChanged()));
	connect(m_ui.tuner_radius, SIGNAL(valueChanged(double)), this, SLOT(tuner_radius_ValueChanged(double)));
	connect(m_ui.tuner_bias, SIGNAL(valueChanged(double)), this, SLOT(tuner_bias_ValueChanged(double)));
	connect(m_ui.btn_auto_detect, SIGNAL(clicked()), this, SLOT(btn_auto_detect_Click()));
	connect(m_ui.chk_force_cull, SIGNAL(toggled(bool)), this, SLOT(chk_force_cull_Checked(bool)));

	QJsonObject att = jobj["attributes"].toObject();	
	if (att.contains("intensity"))
	{
		m_ui.tuner_intensity->setValue(att["intensity"].toString().toDouble());
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

	if (att.contains("target"))
	{
		m_ui.text_target->setText(att["target"].toString());
	}

	if (att.contains("castShadow"))
	{		
		bool cast_shadow = decodeJsonBoolLiteral(att["castShadow"].toString());
		m_ui.chk_cast_shadow->setChecked(cast_shadow);
	}

	if (att.contains("size"))
	{
		QString size = att["size"].toString();		
		auto values = size.split(",");
		m_ui.tuner_width->setValue(values[0].toInt());
		m_ui.tuner_height->setValue(values[1].toInt());
	}

	if (att.contains("area"))
	{
		QString area = att["area"].toString();
		auto values = area.split(",");
		m_ui.tuner_left->setValue(values[0].toDouble());
		m_ui.tuner_right->setValue(values[1].toDouble());
		m_ui.tuner_bottom->setValue(values[2].toDouble());
		m_ui.tuner_top->setValue(values[3].toDouble());
		m_ui.tuner_near->setValue(values[4].toDouble());
		m_ui.tuner_far->setValue(values[5].toDouble());
	}

	if (att.contains("radius"))
	{
		m_ui.tuner_radius->setValue(att["radius"].toString().toDouble());
	}

	if (att.contains("bias"))
	{
		m_ui.tuner_bias->setValue(att["bias"].toString().toDouble());
	}	

	bool force_cull = true;
	if (att.contains("force_cull"))
	{
		force_cull = decodeJsonBoolLiteral(att["force_cull"].toString());
	}
	m_ui.chk_force_cull->setChecked(force_cull);

	obj3d_tuner = new Object3DTuner(this, jobj);
	obj3d_tuner->m_ui.logo->setPixmap(QPixmap(QString(":/images/directional_light.png")));
	obj3d_tuner->m_ui.tuner_rot->setEnabled(false);
	obj3d_tuner->m_ui.tuner_scale->setEnabled(false);
	m_ui.stack->addWidget(obj3d_tuner);
	connect(obj3d_tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner3d_update(QJsonObject)));


	initialized = true;
}

DirectionalLightTuner::~DirectionalLightTuner()
{

}


void DirectionalLightTuner::tuner3d_update(QJsonObject tuning)
{
	jobj = obj3d_tuner->jobj;
	emit update(tuning);
}

void DirectionalLightTuner::tuner_intensity_ValueChanged(double value)
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["intensity"] = QString::number(m_ui.tuner_intensity->value());

	QJsonObject att = jobj["attributes"].toObject();
	att["intensity"] = tuning["intensity"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;

	emit update(tuning);

}

void DirectionalLightTuner::tuner_color_ValueChanged()
{
	float r, g, b;
	m_ui.tuner_color->get_color(r, g, b);

	QJsonObject tuning;
	tuning["color"] = QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b);

	QJsonObject att = jobj["attributes"].toObject();
	att["color"] = tuning["color"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;

	emit update(tuning);
}

void DirectionalLightTuner::update_target()
{
	QJsonObject tuning;
	tuning["target"] = m_ui.text_target->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["target"] = tuning["target"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;

	emit update(tuning);

}

void DirectionalLightTuner::update_shadow()
{
	if (!initialized) return;

	QJsonObject tuning;
	QJsonObject att = jobj["attributes"].toObject();

	tuning["castShadow"] = encodeJsonBoolLiteral(m_ui.chk_cast_shadow->isChecked());
	tuning["size"] = QString::number(m_ui.tuner_width->value()) + ", " + QString::number(m_ui.tuner_height->value());

	att["castShadow"] = tuning["castShadow"];
	att["size"] = tuning["size"];

	if (m_ui.chk_cast_shadow->isChecked())
	{
		tuning["area"] =
			QString::number(m_ui.tuner_left->value()) + ", " + QString::number(m_ui.tuner_right->value()) + ", " +
			QString::number(m_ui.tuner_bottom->value()) + ", " + QString::number(m_ui.tuner_top->value()) + ", " +
			QString::number(m_ui.tuner_near->value()) + ", " + QString::number(m_ui.tuner_far->value());

		tuning["radius"] = QString::number(m_ui.tuner_radius->value());
		tuning["bias"] = QString::number(m_ui.tuner_bias->value());
		tuning["force_cull"] = encodeJsonBoolLiteral(m_ui.chk_force_cull->isChecked());

		att["area"] = tuning["area"];
		att["radius"] = tuning["radius"];
		att["force_cull"] = tuning["force_cull"];
	}
	jobj["attributes"] = att;
	obj3d_tuner->jobj = jobj;

	emit update(tuning);
}

void DirectionalLightTuner::chk_cast_shadow_Checked(bool checked)
{	
	m_ui.grp_shadow->setEnabled(checked);
	if (initialized)
	{
		update_shadow();
	}
}

void DirectionalLightTuner::tuner_area_ValueChanged()
{
	if (!initialized) return;
	QJsonObject tuning;
	QJsonObject att = jobj["attributes"].toObject();	
	
	tuning["area"] =
		QString::number(m_ui.tuner_left->value()) + ", " + QString::number(m_ui.tuner_right->value()) + ", " +
		QString::number(m_ui.tuner_bottom->value()) + ", " + QString::number(m_ui.tuner_top->value()) + ", " +
		QString::number(m_ui.tuner_near->value()) + ", " + QString::number(m_ui.tuner_far->value());
	att["area"] = tuning["area"];

	jobj["attributes"] = att;
	obj3d_tuner->jobj = jobj;

	emit update(tuning);
}

void DirectionalLightTuner::tuner_radius_ValueChanged(double value)
{
	if (!initialized) return;

	QJsonObject tuning;
	QJsonObject att = jobj["attributes"].toObject();

	tuning["radius"] = QString::number(m_ui.tuner_radius->value());
	att["radius"] = tuning["radius"];

	jobj["attributes"] = att;
	obj3d_tuner->jobj = jobj;

	emit update(tuning);

}

void DirectionalLightTuner::tuner_bias_ValueChanged(double value)
{
	if (!initialized) return;

	QJsonObject tuning;
	QJsonObject att = jobj["attributes"].toObject();

	tuning["bias"] = QString::number(m_ui.tuner_bias->value());
	att["bias"] = tuning["bias"];

	jobj["attributes"] = att;
	obj3d_tuner->jobj = jobj;

	emit update(tuning);

}

void DirectionalLightTuner::update_result(QJsonObject ret)
{
	QJsonObject att = jobj["attributes"].toObject();
	att["area"] = ret["area"];
	QString area = ret["area"].toString();
	auto values = area.split(",");
	m_ui.tuner_left->setValue(values[0].toDouble());
	m_ui.tuner_right->setValue(values[1].toDouble());
	m_ui.tuner_bottom->setValue(values[2].toDouble());
	m_ui.tuner_top->setValue(values[3].toDouble());
	m_ui.tuner_near->setValue(values[4].toDouble());
	m_ui.tuner_far->setValue(values[5].toDouble());

	jobj["attributes"] = att;
	obj3d_tuner->jobj = jobj;
}

void DirectionalLightTuner::btn_auto_detect_Click()
{
	QJsonObject tuning;
	tuning["auto_area"] = "auto";	
	emit update(tuning);
}

void DirectionalLightTuner::chk_force_cull_Checked(bool checked)
{
	if (!initialized) return;
	QJsonObject tuning;
	QJsonObject att = jobj["attributes"].toObject();

	tuning["force_cull"] = encodeJsonBoolLiteral(checked);
	att["force_cull"] = tuning["force_cull"];

	jobj["attributes"] = att;
	obj3d_tuner->jobj = jobj;

	emit update(tuning);
}
