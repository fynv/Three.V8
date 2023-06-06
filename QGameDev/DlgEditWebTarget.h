#pragma once

#include <QDialog>
#include <QJsonObject>
#include "ui_DlgEditWebTarget.h"

class DlgEditWebTarget : public QDialog
{
	Q_OBJECT
public:
	QJsonObject jTarget;
	DlgEditWebTarget(QWidget* parent, QString root);
	DlgEditWebTarget(QWidget* parent, const QJsonObject& jTarget, QString root);
	virtual ~DlgEditWebTarget();

	void accept() override;

private slots:
	void text_input_edited(const QString& text);
	void btn_browse_Click();

private:
	Ui_DlgEditWebTarget m_ui;
	QString root;
	void _init();
};

