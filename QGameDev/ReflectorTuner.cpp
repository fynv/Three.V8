#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "ReflectorTuner.h"
#include "JsonUtils.h"

ReflectorTuner::ReflectorTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);
	connect(m_ui.tuner_width, SIGNAL(valueChanged(double)), this, SLOT(tuner_size_ValueChanged()));
	connect(m_ui.tuner_height, SIGNAL(valueChanged(double)), this, SLOT(tuner_size_ValueChanged()));
	connect(m_ui.btn_add, SIGNAL(clicked()), this, SIGNAL(add_ref_prim()));
	connect(m_ui.btn_remove, SIGNAL(clicked()), this, SLOT(remove_ref_prim()));

	QJsonObject att = jobj["attributes"].toObject();
	if (att.contains("size"))
	{
		QString size = att["size"].toString();
		auto values = size.split(",");
		m_ui.tuner_width->setValue(values[0].toDouble());
		m_ui.tuner_height->setValue(values[1].toDouble());
	}

	update_prim_refs();		

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

void ReflectorTuner::update_prim_refs()
{
	m_ui.lst_prim_refs->clear();
	QJsonObject att = jobj["attributes"].toObject();	
	if (att.contains("prim_refs"))
	{
		QString prim_refs = att["prim_refs"].toString();
		QJsonArray arr_prim_refs = QJsonDocument::fromJson(prim_refs.toUtf8()).array();
		for (size_t i = 0; i < arr_prim_refs.size(); i++)
		{
			QJsonObject obj_prim_ref = arr_prim_refs[i].toObject();
			QString name = obj_prim_ref["name"].toString();
			int mesh_id = 0;
			int prim_id = 0;
			if (obj_prim_ref.contains("mesh_id"))
			{
				mesh_id = obj_prim_ref["mesh_id"].toInt();				
			}
			if (obj_prim_ref.contains("prim_id"))
			{
				prim_id = obj_prim_ref["prim_id"].toInt();
			}
			QString row = QString("model: ") + name
				+ QString("  mesh: ") + QString::number(mesh_id)
				+ QString("  prim: ") + QString::number(prim_id);
			m_ui.lst_prim_refs->addItem(row);
		}
	}

}

void ReflectorTuner::prim_ref_picked(QByteArray json_prim_ref)
{
	QJsonObject att = jobj["attributes"].toObject();
	QJsonArray arr_prim_refs;
	if (att.contains("prim_refs"))
	{
		QString prim_refs = att["prim_refs"].toString();
		arr_prim_refs = QJsonDocument::fromJson(prim_refs.toUtf8()).array();
	}

	QJsonObject prim_ref = QJsonDocument::fromJson(json_prim_ref).object();
	arr_prim_refs.append(prim_ref);

	QString prim_refs = QJsonDocument(arr_prim_refs).toJson(QJsonDocument::Compact);	
	att["prim_refs"] = prim_refs;
	jobj["attributes"] = att;
	obj3d_tuner->jobj = jobj;

	update_prim_refs();

	QJsonObject tuning;
	tuning["prim_refs"] = prim_refs;

	emit update(tuning);
}

void ReflectorTuner::remove_ref_prim()
{
	int idx = m_ui.lst_prim_refs->currentRow();
	if (idx < 0) return;

	QJsonObject att = jobj["attributes"].toObject();
	QString prim_refs = att["prim_refs"].toString();
	QJsonArray arr_prim_refs = QJsonDocument::fromJson(prim_refs.toUtf8()).array();
	arr_prim_refs.removeAt(idx);

	prim_refs = QJsonDocument(arr_prim_refs).toJson(QJsonDocument::Compact);
	att["prim_refs"] = prim_refs;
	jobj["attributes"] = att;
	obj3d_tuner->jobj = jobj;

	update_prim_refs();

	QJsonObject tuning;
	tuning["prim_refs"] = prim_refs;		

	emit update(tuning);
}

