#include <QFileDialog>
#include <QMessageBox>
#include <GamePlayer.h>
#include "LODProbeGridTuner.h"
#include "JsonUtils.h"

LODProbeGridTuner::LODProbeGridTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.fn_probe_data, SIGNAL(editingFinished()), this, SLOT(load_data()));
	connect(m_ui.btn_browse, SIGNAL(clicked()), this, SLOT(btn_browse_Click()));
	connect(m_ui.tuner_divisions, SIGNAL(valueChanged()), this, SLOT(tuner_divisions_ValueChanged()));
	connect(m_ui.tuner_coverage_min, SIGNAL(valueChanged()), this, SLOT(tuner_coverage_min_ValueChanged()));
	connect(m_ui.tuner_coverage_max, SIGNAL(valueChanged()), this, SLOT(tuner_coverage_max_ValueChanged()));
	connect(m_ui.tuner_sub_div, SIGNAL(valueChanged(int)), this, SLOT(tuner_sub_div_ValueChanged(int)));
	connect(m_ui.btn_initialize, SIGNAL(clicked()), this, SIGNAL(initialize()));
	connect(m_ui.tuner_normal_bias, SIGNAL(valueChanged(double)), this, SLOT(tuner_normal_bias_ValueChanged(double)));
	connect(m_ui.btn_start, SIGNAL(clicked()), this, SLOT(btn_start_Click()));
	connect(m_ui.btn_auto_detect, SIGNAL(clicked()), this, SLOT(btn_auto_detect_Click()));
	connect(m_ui.chk_per_primitive, SIGNAL(toggled(bool)), this, SLOT(chk_per_primitive_Checked(bool)));

	QJsonObject att = jobj["attributes"].toObject();
	m_ui.fn_probe_data->setText("assets/lod_probes.dat");
	if (att.contains("probe_data"))
	{
		m_ui.fn_probe_data->setText(att["probe_data"].toString());
	}

	m_ui.tuner_divisions->m_ui.tuner_x->setMinimum(2);
	m_ui.tuner_divisions->m_ui.tuner_y->setMinimum(2);
	m_ui.tuner_divisions->m_ui.tuner_z->setMinimum(2);
	m_ui.tuner_divisions->set_value(10, 5, 10);
	if (att.contains("base_divisions"))
	{
		QString divisions = att["base_divisions"].toString();
		auto values = divisions.split(",");
		int x = values[0].toInt();
		int y = values[1].toInt();
		int z = values[2].toInt();
		m_ui.tuner_divisions->set_value(x, y, z);
	}

	m_ui.tuner_coverage_min->set_value(-10.0f, 0.0f, -10.0f);
	if (att.contains("coverage_min"))
	{
		QString coverage_min = att["coverage_min"].toString();
		auto values = coverage_min.split(",");
		float x = values[0].toFloat();
		float y = values[1].toFloat();
		float z = values[2].toFloat();
		m_ui.tuner_coverage_min->set_value(x, y, z);
	}


	m_ui.tuner_coverage_max->set_value(10.0f, 10.0f, 10.0f);
	if (att.contains("coverage_min"))
	{
		QString coverage_max = att["coverage_max"].toString();
		auto values = coverage_max.split(",");
		float x = values[0].toFloat();
		float y = values[1].toFloat();
		float z = values[2].toFloat();
		m_ui.tuner_coverage_max->set_value(x, y, z);
	}

	m_ui.tuner_sub_div->setValue(2);
	if (att.contains("sub_division_level"))
	{
		m_ui.tuner_sub_div->setValue(att["sub_division_level"].toString().toInt());
	}

	m_ui.tuner_normal_bias->setValue(0.2);
	m_ui.tuner_normal_bias->setSingleStep(0.05);
	if (att.contains("normal_bias"))
	{
		m_ui.tuner_normal_bias->setValue(att["normal_bias"].toString().toFloat());
	}

	if (att.contains("per_primitive"))
	{
		bool per_primitive = decodeJsonBoolLiteral(att["per_primitive"].toString());
		m_ui.chk_per_primitive->setChecked(per_primitive);
	}

	m_ui.tuner_iterations->setValue(6);
	m_ui.tuner_num_rays->setMaximum(32768);
	m_ui.tuner_num_rays->setValue(8192);
	m_ui.tuner_num_rays->setSingleStep(2048);

	initialized = true;
}

LODProbeGridTuner::~LODProbeGridTuner()
{

}

void LODProbeGridTuner::update_result(QJsonObject res)
{
	QJsonObject att = jobj["attributes"].toObject();
	if (res.contains("base_divisions"))
	{
		att["base_divisions"] = res["base_divisions"];
		QString divisions = res["base_divisions"].toString();
		auto values = divisions.split(",");
		int x = values[0].toInt();
		int y = values[1].toInt();
		int z = values[2].toInt();
		m_ui.tuner_divisions->set_value(x, y, z);
	}

	if (res.contains("coverage_min"))
	{
		att["coverage_min"] = res["coverage_min"];
		QString coverage_min = res["coverage_min"].toString();
		auto values = coverage_min.split(",");
		float x = values[0].toFloat();
		float y = values[1].toFloat();
		float z = values[2].toFloat();
		m_ui.tuner_coverage_min->set_value(x, y, z);
	}

	if (res.contains("coverage_max"))
	{
		att["coverage_max"] = res["coverage_max"];
		QString coverage_max = res["coverage_max"].toString();
		auto values = coverage_max.split(",");
		float x = values[0].toFloat();
		float y = values[1].toFloat();
		float z = values[2].toFloat();
		m_ui.tuner_coverage_max->set_value(x, y, z);
	}

	if (res.contains("sub_division_level"))
	{
		att["sub_division_level"] = res["sub_division_level"];
		m_ui.tuner_sub_div->setValue(res["sub_division_level"].toString().toInt());
	}

	jobj["attributes"] = att;
}

void LODProbeGridTuner::load_data()
{
	QJsonObject tuning;
	tuning["probe_data"] = m_ui.fn_probe_data->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["probe_data"] = tuning["probe_data"];
	jobj["attributes"] = att;

	emit update(tuning);
}


void LODProbeGridTuner::btn_browse_Click()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Probe Grid"), QString(), tr("Data File(*.dat)"));
	if (filename.isNull()) return;

	QString cur_path = QDir::current().absolutePath();
	if (!filename.startsWith(cur_path))
	{
		QMessageBox::warning(this, tr("Failed to parse path"), tr("File is not in current tree"));
		return;
	}

	QString rel_path = filename.mid(cur_path.length() + 1);
	m_ui.fn_probe_data->setText(rel_path);
	load_data();
}

void LODProbeGridTuner::tuner_divisions_ValueChanged()
{
	if (!initialized) return;

	int x, y, z;
	m_ui.tuner_divisions->get_value(x, y, z);
	QJsonObject tuning;
	tuning["base_divisions"] = QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z);

	QJsonObject att = jobj["attributes"].toObject();
	att["base_divisions"] = tuning["base_divisions"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void LODProbeGridTuner::tuner_coverage_min_ValueChanged()
{
	if (!initialized) return;

	float x, y, z;
	m_ui.tuner_coverage_min->get_value(x, y, z);
	QJsonObject tuning;
	tuning["coverage_min"] = QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z);

	QJsonObject att = jobj["attributes"].toObject();
	att["coverage_min"] = tuning["coverage_min"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void LODProbeGridTuner::tuner_coverage_max_ValueChanged()
{
	if (!initialized) return;

	float x, y, z;
	m_ui.tuner_coverage_max->get_value(x, y, z);
	QJsonObject tuning;
	tuning["coverage_max"] = QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z);

	QJsonObject att = jobj["attributes"].toObject();
	att["coverage_max"] = tuning["coverage_max"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void LODProbeGridTuner::tuner_sub_div_ValueChanged(int value)
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["sub_division_level"] = QString::number(value);

	QJsonObject att = jobj["attributes"].toObject();
	att["sub_division_level"] = tuning["sub_division_level"];
	jobj["attributes"] = att;

	emit update(tuning);

}

void LODProbeGridTuner::initialize_result(QString res)
{	
	m_ui.text_num_probes->setText(tr("Number of probes: ") + res);
}

void LODProbeGridTuner::tuner_normal_bias_ValueChanged(double value)
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["normal_bias"] = QString::number(m_ui.tuner_normal_bias->value());

	QJsonObject att = jobj["attributes"].toObject();
	att["normal_bias"] = tuning["normal_bias"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void LODProbeGridTuner::btn_start_Click()
{
	QJsonObject tuning;
	tuning["iterations"] = QString::number(m_ui.tuner_iterations->value());
	tuning["num_rays"] = QString::number(m_ui.tuner_num_rays->value());
	emit generate(tuning);
}

void LODProbeGridTuner::btn_auto_detect_Click()
{
	QJsonObject tuning;
	tuning["auto_area"] = "auto";
	emit update(tuning);
}

void LODProbeGridTuner::chk_per_primitive_Checked(bool checked)
{
	if (initialized)
	{
		QJsonObject tuning;
		tuning["per_primitive"] = encodeJsonBoolLiteral(m_ui.chk_per_primitive->isChecked());

		QJsonObject att = jobj["attributes"].toObject();
		att["per_primitive"] = tuning["per_primitive"];
		jobj["attributes"] = att;

		emit update(tuning);
	}
}

