#include "DlgNewDir.h"


DlgNewDir::DlgNewDir(QWidget* parent) : QDialog(parent)
{
	m_ui.setupUi(this);
	
}

DlgNewDir::~DlgNewDir()
{

}

void DlgNewDir::accept()
{
	if (m_ui.text_filename->text() == "") return;
	filename = m_ui.text_filename->text();
	QDialog::accept();
}
