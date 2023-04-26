#pragma once

#include <QDialog>
#include <QJsonObject>
#include "ui_DlgProjectSettings.h"

class DlgProjectSettings : public QDialog
{
	Q_OBJECT
public:
	QJsonObject projectData;
	DlgProjectSettings(QWidget* parent, const QJsonObject& projectData);
	virtual ~DlgProjectSettings();

	void accept() override;

private:
	Ui_DlgProjectSettings m_ui;
};
