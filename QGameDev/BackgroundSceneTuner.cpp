#include <GamePlayer.h>
#include <QFileDialog>
#include <QMessageBox>
#include "BackgroundSceneTuner.h"


BackgroundSceneTuner::BackgroundSceneTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.scene_path, SIGNAL(editingFinished()), this, SLOT(text_editing_finished()));
	connect(m_ui.btn_browse, SIGNAL(clicked()), this, SLOT(btn_browse_Click()));
	connect(m_ui.tuner_near, SIGNAL(valueChanged(double)), this, SLOT(tuner_near_ValueChanged(double)));
	connect(m_ui.tuner_far, SIGNAL(valueChanged(double)), this, SLOT(tuner_far_ValueChanged(double)));
	
	QJsonObject att = jobj["attributes"].toObject();

	m_ui.scene_path->setText("terrain.xml");
	if (att.contains("scene"))
	{
		m_ui.scene_path->setText(att["scene"].toString());
	}

	if (att.contains("near"))
	{
		m_ui.tuner_near->setValue(att["near"].toString().toDouble());
	}

	if (att.contains("far"))
	{
		m_ui.tuner_far->setValue(att["far"].toString().toDouble());
	}

	initialized = true;
}

BackgroundSceneTuner::~BackgroundSceneTuner()
{

}

void BackgroundSceneTuner::update_scene()
{
	QJsonObject tuning;
	tuning["scene"] = m_ui.scene_path->text();
	
	QJsonObject att = jobj["attributes"].toObject();
	att["scene"] = tuning["scene"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void BackgroundSceneTuner::text_editing_finished()
{
	update_scene();
}

void BackgroundSceneTuner::btn_browse_Click()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Background Scene"), QString(), tr("XML(*.xml)"));
	if (filename.isNull()) return;

	QString cur_path = QDir::current().absolutePath();
	if (!filename.startsWith(cur_path))
	{
		QMessageBox::warning(this, tr("Failed to parse path"), tr("File is not in current tree"));
		return;
	}

	QString rel_path = filename.mid(cur_path.length() + 1);
	m_ui.scene_path->setText(rel_path);

	update_scene();
}

void BackgroundSceneTuner::tuner_near_ValueChanged(double value)
{
	if (!initialized) return;
	QJsonObject tuning;
	tuning["near"] = QString::number(m_ui.tuner_near->value());

	QJsonObject att = jobj["attributes"].toObject();
	att["near"] = tuning["near"];
	jobj["attributes"] = att;

	emit update(tuning);
}

void BackgroundSceneTuner::tuner_far_ValueChanged(double value)
{
	if (!initialized) return;
	QJsonObject tuning;
	tuning["far"] = QString::number(m_ui.tuner_far->value());

	QJsonObject att = jobj["attributes"].toObject();
	att["far"] = tuning["far"];
	jobj["attributes"] = att;

	emit update(tuning);
}
