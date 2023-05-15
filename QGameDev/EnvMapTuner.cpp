#include <QFileDialog>
#include <QMessageBox>
#include <GamePlayer.h>
#include "EnvMapTuner.h"
#include "JsonUtils.h"

EnvMapTuner::EnvMapTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	m_ui.tuner_probe_pos->m_ui.tuner_x->setSingleStep(0.5);
	m_ui.tuner_probe_pos->m_ui.tuner_y->setSingleStep(0.5);
	m_ui.tuner_probe_pos->m_ui.tuner_z->setSingleStep(0.5);
	connect(m_ui.tuner_probe_pos, SIGNAL(valueChanged()), this, SLOT(tuner_probe_pos_ValueChanged()));
	connect(m_ui.btn_reload, SIGNAL(clicked()), this, SLOT(reload()));
	connect(m_ui.btn_generate, SIGNAL(clicked()), this, SLOT(btn_generate_Click()));
	connect(m_ui.btn_browse, SIGNAL(clicked()), this, SLOT(btn_browse_Click()));
	connect(m_ui.fn_sh, SIGNAL(editingFinished()), this, SLOT(load_sh()));
	connect(m_ui.chk_irr_only, SIGNAL(toggled(bool)), this, SLOT(chk_irr_only_Checked(bool)));

	QJsonObject att = jobj["attributes"].toObject();
	if (att.contains("irradiance_only"))
	{
		bool irradiance_only = decodeJsonBoolLiteral(att["irradiance_only"].toString());
		m_ui.chk_irr_only->setChecked(irradiance_only);
		m_ui.grp_cubemap->setEnabled(!irradiance_only);
		m_ui.grp_sh->setEnabled(irradiance_only);
	}

	m_ui.file_path->setText("assets/textures");
	if (att.contains("path"))
	{
		m_ui.file_path->setText(att["path"].toString());
	}
	m_ui.name_posx->setText("env_face0.jpg");
	if (att.contains("posx"))
	{
		m_ui.name_posx->setText(att["posx"].toString());
	}

	m_ui.name_negx->setText("env_face1.jpg");
	if (att.contains("negx"))
	{
		m_ui.name_negx->setText(att["negx"].toString());
	}

	m_ui.name_posy->setText("env_face2.jpg");
	if (att.contains("posy"))
	{
		m_ui.name_posy->setText(att["posy"].toString());
	}

	m_ui.name_negy->setText("env_face3.jpg");
	if (att.contains("negy"))
	{
		m_ui.name_negy->setText(att["negy"].toString());
	}

	m_ui.name_posz->setText("env_face4.jpg");
	if (att.contains("posz"))
	{
		m_ui.name_posz->setText(att["posz"].toString());
	}

	m_ui.name_negz->setText("env_face5.jpg");
	if (att.contains("negz"))
	{
		m_ui.name_negz->setText(att["negz"].toString());
	}

	m_ui.fn_sh->setText("assets/sh.json");
	if (att.contains("path_sh"))
	{
		m_ui.fn_sh->setText(att["path_sh"].toString());
	}

	if (att.contains("probe_position"))
	{
		QString probe_position = att["probe_position"].toString();		
		auto values = probe_position.split(",");
		float x = values[0].toFloat();
		float y = values[1].toFloat();
		float z = values[2].toFloat();
		m_ui.tuner_probe_pos->set_value(x, y, z);
	}
	initialized = true;
}

EnvMapTuner::~EnvMapTuner()
{

}


void EnvMapTuner::tuner_probe_pos_ValueChanged()
{
	if (!initialized) return;

	float x, y, z;
	m_ui.tuner_probe_pos->get_value(x, y, z);
	QJsonObject tuning;
	tuning["probe_position"] = QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z);

	QJsonObject att = jobj["attributes"].toObject();
	att["probe_position"] = tuning["probe_position"];
	jobj["attributes"] = att;

	emit update(tuning);
}


void EnvMapTuner::reload()
{
	QJsonObject tuning;
	tuning["path"] = m_ui.file_path->text();
	tuning["posx"] = m_ui.name_posx->text();
	tuning["negx"] = m_ui.name_negx->text();
	tuning["posy"] = m_ui.name_posy->text();
	tuning["negy"] = m_ui.name_negy->text();
	tuning["posz"] = m_ui.name_posz->text();
	tuning["negz"] = m_ui.name_negz->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["path"] = tuning["path"];
	att["posx"] = tuning["posx"];
	att["negx"] = tuning["negx"];
	att["posy"] = tuning["posy"];
	att["negy"] = tuning["negy"];
	att["posz"] = tuning["posz"];
	att["negz"] = tuning["negz"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void EnvMapTuner::btn_generate_Click()
{
	QJsonObject tuning;
	tuning["path"] = m_ui.file_path->text();
	tuning["posx"] = m_ui.name_posx->text();
	tuning["negx"] = m_ui.name_negx->text();
	tuning["posy"] = m_ui.name_posy->text();
	tuning["negy"] = m_ui.name_negy->text();
	tuning["posz"] = m_ui.name_posz->text();
	tuning["negz"] = m_ui.name_negz->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["path"] = tuning["path"];
	att["posx"] = tuning["posx"];
	att["negx"] = tuning["negx"];
	att["posy"] = tuning["posy"];
	att["negy"] = tuning["negy"];
	att["posz"] = tuning["posz"];
	att["negz"] = tuning["negz"];
	jobj["attributes"] = att;

	emit generate(tuning);

}

void EnvMapTuner::btn_browse_Click()
{
	QFileDialog dlg;
	dlg.setFileMode(QFileDialog::ExistingFiles);
	dlg.setNameFilter(tr("Images(*.jpg *.png *.hdr)"));
	if (dlg.exec() == 0) return;

	QString cur_path = QDir::current().absolutePath();
	QStringList filenames = dlg.selectedFiles();

	if (filenames.length() < 6)
	{
		QMessageBox::warning(this, tr("Wrong number of files"), tr("Exactly 6 files needed"));
		return;
	}

	QString dir = QFileInfo(filenames[0]).absolutePath();
	if (!dir.startsWith(cur_path))
	{
		QMessageBox::warning(this, tr("Failed to parse path"), tr("Files are not in current tree"));
		return;
	}

	QString rel_dir = dir.mid(cur_path.length() + 1);
	m_ui.file_path->setText(rel_dir);
	m_ui.name_posx->setText(filenames[0].mid(dir.length() + 1));
	m_ui.name_negx->setText(filenames[1].mid(dir.length() + 1));
	m_ui.name_posy->setText(filenames[2].mid(dir.length() + 1));
	m_ui.name_negy->setText(filenames[3].mid(dir.length() + 1));
	m_ui.name_posz->setText(filenames[4].mid(dir.length() + 1));
	m_ui.name_negz->setText(filenames[5].mid(dir.length() + 1));

	reload();
}

void EnvMapTuner::load_sh()
{
	QJsonObject tuning;
	tuning["path_sh"] = m_ui.fn_sh->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["path_sh"] = tuning["path_sh"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void EnvMapTuner::btn_browse_sh_Click()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open SH Json"), QString(), tr("JSON(*.json)"));
	if (filename.isNull()) return;

	QString cur_path = QDir::current().absolutePath();
	if (!filename.startsWith(cur_path))
	{
		QMessageBox::warning(this, tr("Failed to parse path"), tr("File is not in current tree"));
		return;
	}

	QString rel_path = filename.mid(cur_path.length() + 1);
	m_ui.fn_sh->setText(rel_path);
	load_sh();
}

void EnvMapTuner::chk_irr_only_Checked(bool checked)
{
	if (!initialized) return;
	m_ui.grp_cubemap->setEnabled(!checked);
	m_ui.grp_sh->setEnabled(checked);

	QJsonObject tuning;
	tuning["irradiance_only"] = encodeJsonBoolLiteral(checked);

	QJsonObject att = jobj["attributes"].toObject();
	att["irradiance_only"] = tuning["irradiance_only"];
	jobj["attributes"] = att;

	emit update(tuning);
}

