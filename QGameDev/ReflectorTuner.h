#pragma once

#include <QJsonObject>
#include "ui_ReflectorTuner.h"
#include "Tuner.h"
#include "Object3DTuner.h"

class ReflectorTuner : public Tuner
{
	Q_OBJECT
public:
	ReflectorTuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~ReflectorTuner();

	Object3DTuner* obj3d_tuner = nullptr;	

	void prim_ref_picked(QByteArray json_prim_ref);

signals:
	void add_ref_prim();

private slots:
	void tuner3d_update(QJsonObject tuning);	
	void tuner_size_ValueChanged();		
	void remove_ref_prim();

private:
	Ui_ReflectorTuner m_ui;
	bool initialized = false;
	void update_prim_refs();

};

