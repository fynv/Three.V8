#pragma once

#include <QDialog>
#include "ui_DlgNewDir.h"

class DlgNewDir : public QDialog
{
	Q_OBJECT
public:
	DlgNewDir(QWidget* parent);
	virtual ~DlgNewDir();

	QString filename;
	void accept() override;

private:
	Ui_DlgNewDir m_ui;
};
