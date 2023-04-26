#pragma once

#include "Editor.h"
#include "ui_JSEditor.h"

class JSEditor : public Editor
{
	Q_OBJECT
public:
	JSEditor(QWidget* parent, QString file_path);
	virtual ~JSEditor();

	void doc_save_as() override; // async	
	void doc_save() override; // async	
	void doc_close() override; // async
	void doc_refresh() override; // async

	void undo() override;
	void redo() override;
	void comment() override;
	void upper() override;
	void lower() override;
	void find() override;
	void findnext() override;
	void findprev() override;
	void replace() override;
	void gotoline() override;

private:
	Ui_JSEditor m_ui;
	QString file_path;
	void SetText(QString text);

	template <typename TCallback>
	void TextChanged(TCallback callback);

	template <typename TCallback>
	void GetText(TCallback callback);

	void _doc_save(QString filename);
	void _doc_open(QString filename);

private slots:
	void OnNavigationCompleted();
};

#include "JsonUtils.h"

template <typename TCallback>
void JSEditor::TextChanged(TCallback callback)
{
	m_ui.webView->ExecuteScriptAsync("doc_modified()", [this, callback](QString result) {
		bool modified = decodeJsonBoolLiteral(result);
		callback(modified);
	});
}

template <typename TCallback>
void JSEditor::GetText(TCallback callback)
{
	m_ui.webView->ExecuteScriptAsync("doc_get_text()", [this, callback](QString result) {
		QString text = decodeJsonStringLiteral(result);
		callback(text);
	});
}
