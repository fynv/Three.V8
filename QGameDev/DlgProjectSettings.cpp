#include "DlgProjectSettings.h"

DlgProjectSettings::DlgProjectSettings(QWidget* parent, const QJsonObject& projectData) 
	: QDialog(parent)
	, projectData(projectData)
{
	m_ui.setupUi(this);
	m_ui.text_project_name->setText(projectData["project_name"].toString());

}

DlgProjectSettings::~DlgProjectSettings()
{

}

void DlgProjectSettings::accept()
{
	if (m_ui.text_project_name->text() == "") return;
	projectData["project_name"] = m_ui.text_project_name->text();
	QDialog::accept();
}
