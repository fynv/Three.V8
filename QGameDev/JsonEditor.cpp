#include <QFile>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include "JsonEditor.h"
#include "JsonUtils.h"

JsonEditor::JsonEditor(QWidget* parent, QString file_path)
	: Editor(parent)
	, file_path(file_path)
{
	m_ui.setupUi(this);
	connect(m_ui.webView, SIGNAL(navigationCompleted()), this, SLOT(OnNavigationCompleted()));
	QString local_path = QCoreApplication::applicationDirPath();
	QString src = local_path + "/editor/json_editor.html";
	m_ui.webView->initialize(src);

}


JsonEditor::~JsonEditor()
{


}

void JsonEditor::SetText(QString text)
{
	QString para = encodeJsonStringLiteral(text);
	m_ui.webView->ExecuteScriptAsync("doc_set_text(" + para + ")");
}

void JsonEditor::_doc_save(QString filename)
{
	GetText([this, filename](QString text) {
		QFile file(filename);
	file.open(QFile::WriteOnly);
	file.write(text.toUtf8());
	file.close();
	emit doc_save_ret();
		});
}

void JsonEditor::_doc_open(QString filename)
{
	QFile file(filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString text = file.readAll();
	file.close();
	SetText(text);
}

void JsonEditor::doc_save_as()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save as"), file_path, tr("Javascript(*.js)"));
	if (filename.isNull())
	{
		emit doc_save_as_ret("");
		return;
	}
	file_path = filename;
	connect(this, &JsonEditor::doc_save_ret, this, [this]() {
		emit doc_save_as_ret(file_path);
		}, Qt::SingleShotConnection);
	_doc_save(file_path);
}

void JsonEditor::doc_save()
{
	_doc_save(file_path);
}

void JsonEditor::doc_close()
{
	TextChanged([this](bool changed) {
		if (changed)
		{
			int result = QMessageBox::question(this, tr("Save file"), tr("File has been modified. Save it?"), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));
			if (result == QMessageBox::Yes)
			{
				doc_save();
				emit doc_close_ret(true);
			}
			else if (result == QMessageBox::No)
			{
				emit doc_close_ret(true);
			}
			else
			{
				emit doc_close_ret(false);
			}
		}
		else
		{
			emit doc_close_ret(true);
		}
		});
}


void JsonEditor::doc_refresh()
{
	connect(this, &JsonEditor::doc_close_ret, this, [this](bool res) {
		if (res)
		{
			_doc_open(file_path);
		}
	emit doc_refresh_ret();
		}, Qt::SingleShotConnection);
	doc_close();
}


void JsonEditor::OnNavigationCompleted()
{
	_doc_open(file_path);
}

void JsonEditor::undo()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('undo');");
}

void JsonEditor::redo()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('redo');");
}

void JsonEditor::comment()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('togglecomment');");
}

void JsonEditor::upper()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('touppercase');");
}

void JsonEditor::lower()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('tolowercase');");
}

void JsonEditor::find()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('find');");
}

void JsonEditor::findnext()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('findnext');");
}

void JsonEditor::findprev()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('findprevious');");
}

void JsonEditor::replace()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('replace');");
}

void JsonEditor::gotoline()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('gotoline');");
}
