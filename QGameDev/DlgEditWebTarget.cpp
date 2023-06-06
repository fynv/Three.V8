#include <QFileDialog>
#include <QMessageBox>
#include "DlgEditWebTarget.h"

void DlgEditWebTarget::_init()
{
	m_ui.text_name->setText(this->jTarget["name"].toString());
	m_ui.text_input->setText(this->jTarget["input"].toString());	

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

DlgEditWebTarget::DlgEditWebTarget(QWidget* parent, QString root)
	: QDialog(parent), root(root)
{
	m_ui.setupUi(this);
	this->setWindowTitle(tr("Add Web Target"));
	this->jTarget["name"] = "Target";
	this->jTarget["input"] = "index.html";		
	_init();
}

DlgEditWebTarget::DlgEditWebTarget(QWidget* parent, const QJsonObject& jTarget, QString root)
	: QDialog(parent), jTarget(jTarget), root(root)
{
	m_ui.setupUi(this);
	this->setWindowTitle(tr("Edit Web Target"));
	_init();
}

DlgEditWebTarget::~DlgEditWebTarget()
{

}

void DlgEditWebTarget::accept()
{
	if (m_ui.text_name->text() == "" || m_ui.text_input->text() == "") return;
	this->jTarget["name"] = m_ui.text_name->text();
	this->jTarget["input"] = m_ui.text_input->text();

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

void DlgEditWebTarget::text_input_edited(const QString& text)
{
	if (!text.endsWith(".html"))
	{
		int idx = m_ui.text_input->cursorPosition();
		if (text.endsWith(".htm"))
		{
			m_ui.text_input->setText(text + "l");
		}
		else if (text.endsWith(".ht"))
		{
			m_ui.text_input->setText(text + "ml");
		}
		else if (text.endsWith(".h"))
		{
			m_ui.text_input->setText(text + "tml");
		}
		else if (text.endsWith("."))
		{
			m_ui.text_input->setText(text + "html");
		}
		else
		{
			m_ui.text_input->setText(text + ".html");
		}
		m_ui.text_input->setCursorPosition(idx);
	}	

}

void DlgEditWebTarget::btn_browse_Click()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Select HTML"), QString(), tr("HTML(*.html)"));
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

