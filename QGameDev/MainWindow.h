#pragma once

#include <QList>
#include <QMainWindow>
#include <QMenu>
#include "ui_MainWindow.h"

#include "JsonData.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QApplication* app);
	virtual ~MainWindow();

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	Ui_MainWindow m_ui;

	QList<QString> RecentFiles;
	QList<QString> RecentProjects;
	void UpdateRecentFiles();
	void LoadRecentFiles();
	void SaveRecentFiles();
	void AddRecentFile(QString file_path);
	void UpdateRecentProjects();
	void LoadRecentProjects();
	void SaveRecentProjects();
	void AddRecentProject(QString file_path);

	QString cur_path;
	JsonData project;	
	QSet<QString> target_outputs;
	void create_default_project();
	void update_dir(QTreeWidgetItem* item, QString path);
	void update_targets();
	void update_cur_path();
	void change_path(QString path);

	bool no_update = false;
	bool in_background = false;

	void NewFile(QString path_dir);
	void OpenJavaScript(QString file_path);
	void OpenJSON(QString file_path);
	void OpenXML(QString file_path);
	void OpenFile(QString filename);

	void NewDirectory(QString path_dir);

	void AddTarget();
	void RemoveTarget(int idx);
	void EditTarget(int idx);
	void RunTarget(int idx);
	void CreateTargetShortcut(int idx);

	QMap<QString, QWidget*> opened_tabs;
	void AddTabItem(QWidget* item, QString filepath, QString name);

	template<typename TCallback>
	void CloseTab(int idx, TCallback callback);

	void RecursiveClose();

	void SetDirty();
	void SaveTab(int idx);
	void SaveAsTab(int idx);
	void ReopenTab(int idx);

private slots:
	void OnAppStateChange(Qt::ApplicationState state);
	void CommandNewProject();
	void CommandOpenProject();
	void CommandCloseProject();	
	void CommandNewFile();	
	void CommandOpenFile();
	void CommandCloseFile();
	void CommandSave();
	void CommandSaveAs();
	void CommandSaveAll();
	void CommandReopen();
	void CommandReopenAll();
	void CommandExit();
	void CommandUndo();
	void CommandRedo();
	void CommandComment();
	void CommandUpper();
	void CommandLower();
	void CommandFind();
	void CommandFindNext();
	void CommandFindPrev();
	void CommandReplace();
	void CommandGoto();
	void CommandProjectSettings();
	void CommandHelp();
	void CommandAPIDoc();	
	void CommandTutorials();
	void TrashDir(QString path);
	void TreeFilesContextMenu(const QPoint& pos);
	void TreeFilesItemDoubleClick(QTreeWidgetItem* item, int column);
	void ListTargetsContextMenu(const QPoint& pos);
	void ListTargetsItemDoubleClick(QListWidgetItem* item);
	void btn_add_target_Click();
	void btn_remove_target_Click();
	void btn_edit_target_Click();
	void OnClearConsole();
	void CloseTab(int idx);
	void console_std(QString str);
	void console_err(QString str);

};

#include "Editor.h"

template<typename TCallback>
void MainWindow::CloseTab(int idx, TCallback callback)
{
	EditorBase* editor = dynamic_cast<EditorBase*>(m_ui.tabWidgetEditor->widget(idx));
	if (editor != nullptr)
	{
		connect(editor, &EditorBase::doc_close_ret, this, [this, editor, callback](bool closed) {
			if (closed)
			{
				editor->cleanup();
				int idx = m_ui.tabWidgetEditor->indexOf(editor);
				QString filepath = m_ui.tabWidgetEditor->tabToolTip(idx);
				m_ui.tabWidgetEditor->removeTab(idx);
				editor->deleteLater();
				opened_tabs.remove(filepath);
				callback(true);
			}
			callback(false);
		}, Qt::SingleShotConnection);
		editor->doc_close();
	}
	else
	{
		callback(false);
	}
}