#include <QFileDialog>
#include <QMessageBox>
#include "ModelTuner.h"
#include "JsonUtils.h"

ModelTuner::ModelTuner(QWidget* parent, const QJsonObject& jobj, bool is_avatar)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	
	connect(m_ui.chk_is_building, SIGNAL(toggled(bool)), this, SLOT(chk_is_building_Checked(bool)));
	connect(m_ui.fn_source, SIGNAL(editingFinished()), this, SLOT(load_model()));
	connect(m_ui.btn_browse, SIGNAL(clicked()), this, SLOT(btn_browse_Click()));
	connect(m_ui.fn_lightmap, SIGNAL(editingFinished()), this, SLOT(set_lightmap()));
	connect(m_ui.btn_browse_lightmap, SIGNAL(clicked()), this, SLOT(btn_browse_lightmap_Click()));

	QJsonObject att = jobj["attributes"].toObject();

	m_ui.fn_source->setText("assets/models/model.glb");
	if (att.contains("src"))
	{
		m_ui.fn_source->setText(att["src"].toString());
	}

	if (att.contains("is_building"))
	{
		bool is_building = decodeJsonBoolLiteral(att["is_building"].toString());
		m_ui.chk_is_building->setChecked(is_building);
	}

	m_ui.fn_lightmap->setText("");
	if (att.contains("lightmap"))
	{
		m_ui.fn_lightmap->setText(att["lightmap"].toString());
	}

	obj3d_tuner = new Object3DTuner(this, jobj);
	if (is_avatar)
	{
		obj3d_tuner->m_ui.logo->setPixmap(QPixmap(QString(":/images/avatar.png")));
	}
	else
	{
		obj3d_tuner->m_ui.logo->setPixmap(QPixmap(QString(":/images/model.png")));
	}
	m_ui.stack->addWidget(obj3d_tuner);
	connect(obj3d_tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner3d_update(QJsonObject)));

	initialized = true;
}

ModelTuner::~ModelTuner()
{

}

void ModelTuner::tuner3d_update(QJsonObject tuning)
{
	jobj = obj3d_tuner->jobj;	
	emit update(tuning);
}

void ModelTuner::load_model()
{
	QJsonObject tuning;
	tuning["src"] = m_ui.fn_source->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["src"] = tuning["src"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;

	emit update(tuning);
}

void ModelTuner::btn_browse_Click()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Model"), QString(), tr("GLB Model(*.glb)"));
	if (filename.isNull()) return;

	QString cur_path = QDir::current().absolutePath();
	if (!filename.startsWith(cur_path))
	{
		QMessageBox::warning(this, tr("Failed to parse path"), tr("File is not in current tree"));
		return;
	}

	QString rel_path = filename.mid(cur_path.length() + 1);
	m_ui.fn_source->setText(rel_path);
	load_model();
}

void ModelTuner::chk_is_building_Checked(bool checked)
{
	if (!initialized) return;

	QJsonObject tuning;
	tuning["is_building"] = encodeJsonBoolLiteral(checked);

	QJsonObject att = jobj["attributes"].toObject();
	att["is_building"] = tuning["is_building"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;

	emit update(tuning);
}

void ModelTuner::set_lightmap()
{
	QJsonObject tuning;
	tuning["lightmap"] = m_ui.fn_lightmap->text();

	QJsonObject att = jobj["attributes"].toObject();
	att["lightmap"] = tuning["lightmap"];
	jobj["attributes"] = att;

	obj3d_tuner->jobj = jobj;

	emit update(tuning);
}

void ModelTuner::btn_browse_lightmap_Click()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Lightmap"), QString(), tr("HDR Image(*.hdr);;DDS Image(*.dds);;List(*.csv);;RGBM(*.webp *.png)"));
	if (filename.isNull()) return;

	QString cur_path = QDir::current().absolutePath();
	if (!filename.startsWith(cur_path))
	{
		QMessageBox::warning(this, tr("Failed to parse path"), tr("File is not in current tree"));
		return;
	}

	QString rel_path = filename.mid(cur_path.length() + 1);
	m_ui.fn_lightmap->setText(rel_path);
	set_lightmap();
}
