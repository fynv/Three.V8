#pragma once

#include <QDialog>
#include "ui_DlgNewFile.h"

class DlgNewFile : public QDialog
{
	Q_OBJECT
public:
	DlgNewFile(QWidget* parent);
	virtual ~DlgNewFile();

	QString filetype;
	QString filename;
	void accept() override;

private slots:
	void ListView_SelectionChanged(int row);


private:
	Ui_DlgNewFile m_ui;
};
