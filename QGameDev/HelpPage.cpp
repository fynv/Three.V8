#include "HelpPage.h"

HelpPage::HelpPage(QWidget* parent, QString path) : EditorBase(parent)
{
	m_ui.setupUi(this);
	m_ui.webView->initialize(path);	
}


HelpPage::~HelpPage()
{


}

void HelpPage::Goto(QString path)
{
	m_ui.webView->navigate(path);

}

