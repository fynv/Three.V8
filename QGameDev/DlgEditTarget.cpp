#include <QFileDialog>
#include <QMessageBox>
#include "DlgEditTarget.h"

void DlgEditTarget::_init()
{
	m_ui.text_name->setText(this->jTarget["name"].toString());
	m_ui.text_input->setText(this->jTarget["input"].toString());
	m_ui.text_output->setText(this->jTarget["output"].toString());
	
	if (jTarget.contains("width"))
	{		
		m_ui.text_width->setText(QString::number(this->jTarget["width"].toInt()));
	}

	if (jTarget.contains("height"))
	{		
		m_ui.text_height->setText(QString::number(this->jTarget["height"].toInt()));
	}

	connect(m_ui.text_input, SIGNAL(textEdited(const QString&)), this, SLOT(text_input_edited(const QString&)));
	connect(m_ui.btn_browse, SIGNAL(clicked()), this, SLOT(btn_browse_Click()));
}

DlgEditTarget::DlgEditTarget(QWidget* parent, QString root)
	: QDialog(parent), root(root)
{
	m_ui.setupUi(this);
	this->setWindowTitle("Add Target");
	this->jTarget["name"] = "Target";
	this->jTarget["input"] = "index.js";
	this->jTarget["output"] = "bundle_index.js";
	this->jTarget["dirty"] = true;
	_init();
}

DlgEditTarget::DlgEditTarget(QWidget* parent, const QJsonObject& jTarget, QString root) 
	: QDialog(parent), jTarget(jTarget), root(root)
{
	m_ui.setupUi(this);	
	this->setWindowTitle("Edit Target");
	_init();
}

DlgEditTarget::~DlgEditTarget()
{

}

void DlgEditTarget::accept()
{
	if (m_ui.text_name->text() == "" || m_ui.text_input->text() == "") return;
	this->jTarget["name"] = m_ui.text_name->text();
	this->jTarget["input"] = m_ui.text_input->text();
	this->jTarget["output"] = m_ui.text_output->text();

	int width = m_ui.text_width->text().toInt();
	if (width > 0)
	{
		this->jTarget["width"] = width;
	}
	else if (this->jTarget.contains("width"))
	{
		this->jTarget.remove("width");
	}

	int height = m_ui.text_height->text().toInt();
	if (height > 0)
	{
		this->jTarget["height"] = height;
	}
	else if (this->jTarget.contains("height"))
	{
		this->jTarget.remove("height");
	}

	QDialog::accept();

}

void DlgEditTarget::text_input_edited(const QString& text)
{
	if (!text.endsWith(".js"))
	{
		int idx = m_ui.text_input->cursorPosition();
		if (text.endsWith(".j"))
		{
			m_ui.text_input->setText(text + "s");
		}
		else if (text.endsWith("."))
		{
			m_ui.text_input->setText(text + "js");
		}
		else
		{
			m_ui.text_input->setText(text + ".js");
		}
		m_ui.text_input->setCursorPosition(idx);
	}
	QString new_text = m_ui.text_input->text();
	m_ui.text_output->setText("bundle_" + new_text);

}

void DlgEditTarget::btn_browse_Click()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Select Input Script"), QString(), tr("JavaScript(*.js)"));
	if (filename.isNull()) return;
	if (filename.startsWith(root))
	{
		QString text = filename.mid(root.length() + 1);
		m_ui.text_input->setText(text);
		text_input_edited(text);
	}
	else
	{
		QMessageBox::information(this, tr("Wrong File"), tr("File out of project scope: ") + filename);
	}
}

