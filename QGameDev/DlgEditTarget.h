#pragma once

#include <QDialog>
#include <QJsonObject>
#include "ui_DlgEditTarget.h"

class DlgEditTarget : public QDialog
{
	Q_OBJECT
public:
	QJsonObject jTarget;
	DlgEditTarget(QWidget* parent, QString root);
	DlgEditTarget(QWidget* parent, const QJsonObject& jTarget, QString root);
	virtual ~DlgEditTarget();

	void accept() override;

private slots:
	void text_input_edited(const QString& text);
	void btn_browse_Click();

private:
	Ui_DlgEditTarget m_ui;
	QString root;
	void _init();
};

