#include "DlgNewFile.h"

DlgNewFile::DlgNewFile(QWidget* parent) : QDialog(parent)
{
	m_ui.setupUi(this);
	connect(m_ui.lst_types, SIGNAL(currentRowChanged(int)), this, SLOT(ListView_SelectionChanged(int)));
}

DlgNewFile::~DlgNewFile()
{

}

void DlgNewFile::accept()
{
	if (m_ui.lst_types->currentRow() < 0) return;
	if (m_ui.text_filename->text() == "") return;

	static QList<QString> file_types = { "js", "xml", "json" };
	filetype = file_types[m_ui.lst_types->currentRow()];
	filename = m_ui.text_filename->text();
	QDialog::accept();
}

void DlgNewFile::ListView_SelectionChanged(int row)
{
	if (m_ui.lst_types->currentRow() < 0) return;
	static QList<QString> defalt_filenames = { "index.js", "scene.xml", "data.json" };
	m_ui.text_filename->setText(defalt_filenames[m_ui.lst_types->currentRow()]);
}
