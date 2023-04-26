#pragma once

#include "Editor.h"
#include "ui_HelpPage.h"

class HelpPage : public EditorBase
{
	Q_OBJECT
public:
	HelpPage(QWidget* parent, QString path);
	virtual ~HelpPage();

	void Goto(QString path);

private:
	Ui_HelpPage m_ui;

};
