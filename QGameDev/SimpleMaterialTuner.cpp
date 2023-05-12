#include <QFileDialog>
#include <QMessageBox>
#include "SimpleMaterialTuner.h"

SimpleMaterialTuner::SimpleMaterialTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.tuner_color, SIGNAL(valueChanged()), this, SLOT(tuner_color_ValueChanged()));
	connect(m_ui.fn_texture, SIGNAL(editingFinished()), this, SLOT(load_texture()));
	connect(m_ui.btn_browse, SIGNAL(clicked()), this, SLOT(btn_browse_Click()));
	connect(m_ui.tuner_metalness, SIGNAL(valueChanged(double)), this, SLOT(tuner_metalness_ValueChanged(double)));
	connect(m_ui.tuner_roughness, SIGNAL(valueChanged(double)), this, SLOT(tuner_roughness_ValueChanged(double)));

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

	if (att.contains("texture"))
	{
		m_ui.fn_texture->setText(att["texture"].toString());
	}

	if (att.contains("metalness"))
	{
		m_ui.tuner_metalness->setValue(att["metalness"].toString().toDouble());
	}

	if (att.contains("roughness"))
	{
		m_ui.tuner_roughness->setValue(att["roughness"].toString().toDouble());
	}

}

SimpleMaterialTuner::~SimpleMaterialTuner()
{


}

void SimpleMaterialTuner::tuner_color_ValueChanged()
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

void SimpleMaterialTuner::load_texture()
{
	QJsonObject tuning;
	tuning["texture"] = m_ui.fn_texture->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["texture"] = tuning["texture"];
	jobj["attributes"] = att;

	emit update(tuning);
}


void SimpleMaterialTuner::btn_browse_Click()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Texture"), QString(), tr("Images File(*.jpg *.png)"));
	if (filename.isNull()) return;

	QString cur_path = QDir::current().absolutePath();
	if (!filename.startsWith(cur_path))
	{
		QMessageBox::warning(this, tr("Failed to parse path"), tr("File is not in current tree"));
		return;
	}

	QString rel_path = filename.mid(cur_path.length() + 1);
	m_ui.fn_texture->setText(rel_path);
	load_texture();
}

void SimpleMaterialTuner::tuner_metalness_ValueChanged(double value)
{
	QJsonObject tuning;
	tuning["metalness"] = QString::number(value);

	QJsonObject att = jobj["attributes"].toObject();
	att["metalness"] = tuning["metalness"];
	jobj["attributes"] = att;

	emit update(tuning);

}

void SimpleMaterialTuner::tuner_roughness_ValueChanged(double value)
{
	QJsonObject tuning;
	tuning["roughness"] = QString::number(value);

	QJsonObject att = jobj["attributes"].toObject();
	att["roughness"] = tuning["roughness"];
	jobj["attributes"] = att;

	emit update(tuning);
}

