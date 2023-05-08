#include <QFileDialog>
#include <QMessageBox>
#include <GamePlayer.h>
#include "CubeSkyTuner.h"

CubeSkyTuner::CubeSkyTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.btn_reload, SIGNAL(clicked()), this, SLOT(reload()));
	connect(m_ui.btn_browse, SIGNAL(clicked()), this, SLOT(btn_browse_Click()));

	QJsonObject att = jobj["attributes"].toObject();

	m_ui.file_path->setText("assets/textures");
	if (att.contains("path"))
	{		
		m_ui.file_path->setText(att["path"].toString());
	}

	m_ui.name_posx->setText("face0.jpg");
	if (att.contains("posx"))
	{		
		m_ui.name_posx->setText(att["posx"].toString());
	}

	m_ui.name_negx->setText("face1.jpg");
	if (att.contains("negx"))
	{
		m_ui.name_negx->setText(att["negx"].toString());
	}

	m_ui.name_posy->setText("face2.jpg");
	if (att.contains("posy"))
	{
		m_ui.name_posy->setText(att["posy"].toString());
	}

	m_ui.name_negy->setText("face3.jpg");
	if (att.contains("negy"))
	{
		m_ui.name_negy->setText(att["negy"].toString());
	}

	m_ui.name_posz->setText("face4.jpg");
	if (att.contains("posz"))
	{
		m_ui.name_posz->setText(att["posz"].toString());
	}

	m_ui.name_negz->setText("face5.jpg");
	if (att.contains("negz"))
	{
		m_ui.name_negz->setText(att["negz"].toString());
	}
}

CubeSkyTuner::~CubeSkyTuner()
{

}

void CubeSkyTuner::reload()
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

void CubeSkyTuner::btn_browse_Click()
{
	QFileDialog dlg;
	dlg.setFileMode(QFileDialog::ExistingFiles);
	dlg.setNameFilter(tr("Images(*.jpg *.png)"));
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

