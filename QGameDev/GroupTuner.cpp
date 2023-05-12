#include "GroupTuner.h"


GroupTuner::GroupTuner(QWidget* parent, const QJsonObject& jobj)
	: Tuner(parent, jobj)
{
	m_ui.setupUi(this);

	obj3d_tuner = new Object3DTuner(this, jobj);
	obj3d_tuner->m_ui.logo->setPixmap(QPixmap(QString(":/images/group.png")));
	m_ui.stack->addWidget(obj3d_tuner);
	connect(obj3d_tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner_update(QJsonObject)));
	
}

GroupTuner::~GroupTuner()
{

}

void GroupTuner::tuner_update(QJsonObject tuning)
{
	jobj = obj3d_tuner->jobj;
	emit update(tuning);
}
