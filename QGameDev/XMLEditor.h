#pragma once

#include "Editor.h"
#include "ui_XMLEditor.h"
#include <QJsonObject>
#include <GamePlayer.h>
#include "Tuner.h"

class XMLEditor : public Editor
{
	Q_OBJECT
public:
	XMLEditor(QWidget* parent, QString file_path, QString resource_root);
	virtual ~XMLEditor();

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

signals:
	void output(QString text);
	void error(QString text);

private:
	Ui_XMLEditor m_ui;
	bool changed_cache = false;
	QString text_cache = "";
	int cur_tab = 0;

	QString file_path;
	QString resource_root;
	std::unique_ptr<GamePlayer> m_game_player;

	QJsonObject index;
	QMap<QString, QTreeWidgetItem*> TreeItemMap;
	Tuner* tuner = nullptr;
	QString picked_key = "";

	QTimer press_timer;
	int x_down, y_down;
	
	template <typename TCallback>
	void SetText_code(QString text, TCallback callback);
	void SetText_gl(QString text);
	void SetText(QString text);

	template <typename TCallback>
	void TextChanged_code(TCallback callback);
	bool TextChanged_gl();
	template <typename TCallback>
	void TextChanged(TCallback callback);

	template <typename TCallback>
	void GetText_code(TCallback callback);
	QString GetText_gl();
	template <typename TCallback>
	void GetText(TCallback callback);

	void _doc_save(QString filename);
	void _doc_open(QString filename);

	static void print_std(void* p_self, const char* cstr);
	static void err_std(void* p_self, const char* cstr);

	void update_index_item(QTreeWidgetItem* item, const QJsonObject& obj);
	void update_index();

	std::string index_loaded(const char* json_str);
	std::string object_picked(const char* key);
	std::string object_created(const char* json_str);
	std::string object_removed(const char* key);
	void prim_ref_picked(const char* json_prim_ref);
	static std::string user_message_callback(void* ptr, const char* name, const char* msg);

	void req_create_scene_obj(QString tag);
	void req_create_obj3d(QString tag);

private slots:
	void OnInit();
	void OnPaint(int width, int height);

	void OnMouseDown(QMouseEvent* event);
	void OnMouseUp(QMouseEvent* event);
	void OnMouseMove(QMouseEvent* event);
	void OnWheel(QWheelEvent* event);

	void OnLongPress();

	void OnChar(int charCode);
	void OnControlKey(int code);

	void OnNavigationCompleted();

	void tab_SelectionChanged(int idx);
	void btn_apply_Click();
	void btn_picking_toggled(bool checked);
	void scene_graph_current_changed(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void scene_graph_context_menu(const QPoint& pos);

	void btn_create_fog_Click();
	void btn_create_sky_Click();
	void btn_create_env_light_Click();
	void btn_create_group_Click();
	void btn_create_plane_Click();
	void btn_create_box_Click();
	void btn_create_sphere_Click();
	void btn_create_model_Click();
	void btn_create_directional_light_Click();
	void btn_create_reflector();

	void tuner_update(QJsonObject tuning);
	void tuner_generate(QJsonObject tuning);
	void tuner_initialize();
	void add_prim_ref();


};

#include "JsonUtils.h"

template <typename TCallback>
void XMLEditor::SetText_code(QString text, TCallback callback)
{
	QString para = encodeJsonStringLiteral(text);
	m_ui.webView->ExecuteScriptAsync("doc_set_text(" + para + ")", [callback](QString result) {
		callback();
	});
}


template <typename TCallback>
void XMLEditor::TextChanged_code(TCallback callback)
{
	m_ui.webView->ExecuteScriptAsync("doc_modified()", [this, callback](QString result) {
		bool modified = decodeJsonBoolLiteral(result);
		callback(modified);
	});
}


template <typename TCallback>
void XMLEditor::TextChanged(TCallback callback)
{
	if (changed_cache)
	{
		callback(true);
		return;
	}
	if (cur_tab == 0)
	{
		TextChanged_code(callback);
	}
	else
	{
		callback(TextChanged_gl());
	}
}


template <typename TCallback>
void XMLEditor::GetText_code(TCallback callback)
{
	TextChanged_code([this, callback](bool changed) {
		if (changed)
		{
			m_ui.webView->ExecuteScriptAsync("doc_get_text()", [this, callback](QString result) {
				text_cache = decodeJsonStringLiteral(result);
				changed_cache = true;
				callback(text_cache);
			});
		}
		else
		{
			callback(text_cache);
		}
	});
}


template <typename TCallback>
void XMLEditor::GetText(TCallback callback)
{
	if (cur_tab == 0)
	{
		GetText_code([this, callback](QString text) {
			SetText_gl(text);
			changed_cache = false;
			callback(text);
		});
	}
	else
	{
		QString text = GetText_gl();
		changed_cache = false;
		callback(text);
	}	
}
