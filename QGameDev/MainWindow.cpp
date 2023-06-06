#include <atlbase.h>
#include "shobjidl.h"
#include <shlguid.h>
#include <shlobj_core.h>
#include <vector>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QCloseEvent>
#include "MainWindow.h"
#include "DlgNewFile.h"
#include "DlgNewDir.h"
#include "DlgEditTarget.h"
#include "DlgEditWebTarget.h"
#include "DlgProjectSettings.h"
#include "HelpPage.h"
#include "JSEditor.h"
#include "JsonEditor.h"
#include "HtmlEditor.h"
#include "XMLEditor.h"

void MainWindow::UpdateRecentFiles()
{
	m_ui.menuRecent_Files->clear();
	if (RecentFiles.size() < 1)
	{
		QAction* item = new QAction(this);
		item->setEnabled(false);
		item->setText(tr("None"));
		m_ui.menuRecent_Files->addAction(item);
		return;
	}

	for (int i = 0; i < (int)RecentFiles.size(); i++)
	{
		QString filename = RecentFiles[i];
		QAction* item = new QAction(this);
		item->setText(QString::number(i + 1) + " " + filename);
		item->setStatusTip(filename);	
		
		connect(item, &QAction::triggered, [this, filename]() {
			OpenFile(filename);
		});

		m_ui.menuRecent_Files->addAction(item);
	}
}

void MainWindow::LoadRecentFiles()
{
	RecentFiles.clear();
	QString reg_path = "HKEY_CURRENT_USER\\Software\\GameDev\\recent_files";
	QSettings reg(reg_path, QSettings::NativeFormat);
	for (int i = 0; i < 10; i++)
	{
		QString key = QString::number(i);	
		if (!reg.contains(key)) break;
		QString filename = reg.value(key).toString();
		if (filename == "") break;
		RecentFiles.push_back(filename);
	}
	UpdateRecentFiles();
}

void MainWindow::SaveRecentFiles()
{
	QString reg_path = "HKEY_CURRENT_USER\\Software\\GameDev\\recent_files";
	QSettings reg(reg_path, QSettings::NativeFormat);
	for (int i = 0; i < (int)RecentFiles.size(); i++)
	{
		QString key = QString::number(i);
		QString filename = RecentFiles[i];
		reg.setValue(key, filename);
	}
}

void MainWindow::AddRecentFile(QString file_path)
{
	RecentFiles.insert(0, file_path);
	for (int i = 1; i < (int)RecentFiles.size(); i++)
	{
		if (RecentFiles[i] == file_path || i >= 10)
		{
			RecentFiles.removeAt(i);
			i--;
		}
	}
	UpdateRecentFiles();
	SaveRecentFiles();
}

void MainWindow::UpdateRecentProjects()
{
	m_ui.menuRecent_Projects->clear();
	if (RecentProjects.size() < 1)
	{
		QAction* item = new QAction(this);
		item->setEnabled(false);
		item->setText(tr("None"));
		m_ui.menuRecent_Projects->addAction(item);
		return;
	}

	for (int i = 0; i < (int)RecentProjects.size(); i++)
	{
		QString filename = RecentProjects[i];
		QString path = QFileInfo(filename).path();
		QAction* item = new QAction(this);
		item->setText(QString::number(i + 1) + " " + filename);
		item->setStatusTip(filename);

		connect(item, &QAction::triggered, [this, path]() {
			change_path(path);
		});

		m_ui.menuRecent_Projects->addAction(item);
	}
}

void MainWindow::LoadRecentProjects()
{
	RecentProjects.clear();
	QString reg_path = "HKEY_CURRENT_USER\\Software\\GameDev\\recent_projects";
	QSettings reg(reg_path, QSettings::NativeFormat);
	for (int i = 0; i < 10; i++)
	{
		QString key = QString::number(i);
		if (!reg.contains(key)) break;
		QString filename = reg.value(key).toString();
		if (filename == "") break;
		RecentProjects.push_back(filename);
	}
	UpdateRecentProjects();
}

void MainWindow::SaveRecentProjects()
{
	QString reg_path = "HKEY_CURRENT_USER\\Software\\GameDev\\recent_projects";
	QSettings reg(reg_path, QSettings::NativeFormat);
	for (int i = 0; i < (int)RecentProjects.size(); i++)
	{
		QString key = QString::number(i);
		QString filename = RecentProjects[i];
		reg.setValue(key, filename);
	}
}

void MainWindow::AddRecentProject(QString file_path)
{
	RecentProjects.insert(0, file_path);
	for (int i = 1; i < (int)RecentProjects.size(); i++)
	{
		if (RecentProjects[i] == file_path || i >= 10)
		{
			RecentProjects.removeAt(i);
			i--;
		}
	}
	UpdateRecentProjects();
	SaveRecentProjects();
}

void MainWindow::create_default_project()
{
	project.filename = cur_path + "\\project.json";
	if (!QFile::exists(project.filename))
	{
		QString project_name = QFileInfo(cur_path).fileName();
		QJsonObject obj;
		obj["project_name"] = project_name;
		project.data.setObject(obj);
		project.Save();
	}
}

void MainWindow::update_dir(QTreeWidgetItem* item, QString path)
{
	if (path == "") return;	

	QDir dir(path);
	dir.setFilter(QDir::Dirs| QDir::NoDotAndDotDot);
	QStringList list_dirs = dir.entryList();

	for (size_t i = 0; i < list_dirs.size(); i++)
	{
		QString dir_name = list_dirs[i];
		QString dir = QFileInfo(path + "\\" + dir_name).absoluteFilePath();
		
		QTreeWidgetItem* subitem= new QTreeWidgetItem(item);
		subitem->setText(0, dir_name);
		subitem->setIcon(0, QIcon(":/images/folder.png"));
		subitem->setToolTip(0,dir);
		update_dir(subitem, dir);
	}

	dir.setFilter(QDir::Files);
	QStringList list_files = dir.entryList();

	for (size_t i = 0; i < list_files.size(); i++)
	{
		QString file_name = list_files[i];
		QString file = QFileInfo(path + "\\" + file_name).absoluteFilePath();
		QString ext = QFileInfo(file_name).suffix().toLower();
		if (ext == "js")
		{	
			QString rel_path = file.mid(cur_path.length() + 1);
			if (target_outputs.contains(rel_path)) continue;
		}

		QString icon_name = "doc.png";
		if (ext == "js") icon_name = "js.png";
		if (ext == "xml") icon_name = "xml.png";
		if (ext == "json") icon_name = "json.png";
		if (ext == "html") icon_name = "html.png";

		QTreeWidgetItem* subitem = new QTreeWidgetItem(item);
		subitem->setText(0, file_name);
		subitem->setIcon(0, QIcon(QString(":/images/")+ icon_name));
		subitem->setToolTip(0, file);
	}
}


void MainWindow::update_targets()
{
	m_ui.lst_targets->clear();
	target_outputs.clear();
	if (!project.data.isObject()) return;
	QJsonObject obj_proj = project.data.object();
	if (!obj_proj.contains("targets")) return;
	QJsonArray jTargets = obj_proj["targets"].toArray();

	for (size_t i = 0; i < jTargets.size(); i++)
	{
		QJsonObject jTarget = jTargets[i].toObject();
		QString name = jTarget["name"].toString();
		QString output = jTarget["output"].toString();
		QListWidgetItem* subitem = new QListWidgetItem(m_ui.lst_targets);
		subitem->setText(name);
		subitem->setIcon(QIcon(":/images/target.png"));
		subitem->setToolTip(output);
		target_outputs.insert(output);
	}
}

void MainWindow::update_cur_path()
{
	if (cur_path == "")
	{
		project.Clear();
		this->setWindowTitle(tr("GameDev"));
	}
	else
	{
		QDir::setCurrent(cur_path);
		if (!QFile::exists(cur_path + "\\project.json"))
		{
			create_default_project();
		}
		else
		{
			project.filename = cur_path + "\\project.json";
			project.Load();
			AddRecentProject(project.filename);
		}
		this->setWindowTitle(QString("GameDev - ") + project.data.object()["project_name"].toString());
	}
	update_targets();
	m_ui.tree_files->clear();
	update_dir(m_ui.tree_files->invisibleRootItem(), cur_path);
}


void MainWindow::change_path(QString path)
{		
	QDir dir(path);
	if (path!="" && !dir.exists()) return;	
	QString reg_path = "HKEY_CURRENT_USER\\Software\\GameDev";
	QSettings reg(reg_path, QSettings::NativeFormat);
	reg.setValue("cur_path", path);
	cur_path = path;
	update_cur_path();

}

MainWindow::MainWindow(QApplication* app)
{
	m_ui.setupUi(this);
	m_ui.consoleTextEdit->addAction(m_ui.actionClearConsole);

	connect(app, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(OnAppStateChange(Qt::ApplicationState)));
	connect(m_ui.actionNewProject, SIGNAL(triggered()), this, SLOT(CommandNewProject()));
	connect(m_ui.actionOpenProject, SIGNAL(triggered()), this, SLOT(CommandOpenProject()));
	connect(m_ui.actionClose_Project, SIGNAL(triggered()), this, SLOT(CommandCloseProject()));	
	connect(m_ui.actionNewFile, SIGNAL(triggered()), this, SLOT(CommandNewFile()));
	connect(m_ui.actionOpenFile, SIGNAL(triggered()), this, SLOT(CommandOpenFile()));
	connect(m_ui.action_Close, SIGNAL(triggered()), this, SLOT(CommandCloseFile()));
	connect(m_ui.action_Save, SIGNAL(triggered()), this, SLOT(CommandSave()));
	connect(m_ui.actionSave_As, SIGNAL(triggered()), this, SLOT(CommandSaveAs()));
	connect(m_ui.actionSave_All, SIGNAL(triggered()), this, SLOT(CommandSaveAll()));
	connect(m_ui.action_Reopen, SIGNAL(triggered()), this, SLOT(CommandReopen()));
	connect(m_ui.actionReopen_All, SIGNAL(triggered()), this, SLOT(CommandReopenAll()));
	connect(m_ui.action_Exit, SIGNAL(triggered()), this, SLOT(CommandExit()));

	connect(m_ui.action_Undo, SIGNAL(triggered()), this, SLOT(CommandUndo()));
	connect(m_ui.action_Redo, SIGNAL(triggered()), this, SLOT(CommandRedo()));
	connect(m_ui.actionToggle_Comment, SIGNAL(triggered()), this, SLOT(CommandComment()));
	connect(m_ui.actionChange_to_Upper_Case, SIGNAL(triggered()), this, SLOT(CommandUpper()));
	connect(m_ui.actionChange_to_Lower_Case, SIGNAL(triggered()), this, SLOT(CommandLower()));

	connect(m_ui.action_Find, SIGNAL(triggered()), this, SLOT(CommandFind()));
	connect(m_ui.actionFind_Next, SIGNAL(triggered()), this, SLOT(CommandFindNext()));
	connect(m_ui.actionFind_Previous, SIGNAL(triggered()), this, SLOT(CommandFindPrev()));
	connect(m_ui.action_Replace, SIGNAL(triggered()), this, SLOT(CommandReplace()));
	connect(m_ui.actionGo_to_Line, SIGNAL(triggered()), this, SLOT(CommandGoto()));	

	connect(m_ui.actionSettings, SIGNAL(triggered()), this, SLOT(CommandProjectSettings()));

	connect(m_ui.action_View_Help, SIGNAL(triggered()), this, SLOT(CommandHelp()));
	connect(m_ui.actionThree_V8_API, SIGNAL(triggered()), this, SLOT(CommandAPIDoc()));
	connect(m_ui.actionTutorials, SIGNAL(triggered()), this, SLOT(CommandTutorials()));
	
	connect(m_ui.tree_files, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(TreeFilesContextMenu(const QPoint&)));
	connect(m_ui.tree_files, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(TreeFilesItemDoubleClick(QTreeWidgetItem*, int)));

	{
		QMenu* menu = new QMenu();

		QAction* action_add_target= new QAction(tr("Add Target"));
		menu->addAction(action_add_target);
		connect(action_add_target, &QAction::triggered, [this]() {
			if (!project.data.isObject()) return;
			AddTarget();
		});

		QAction* action_add_web_target = new QAction(tr("Add Web Target"));
		menu->addAction(action_add_web_target);
		connect(action_add_web_target, &QAction::triggered, [this]() {
			if (!project.data.isObject()) return;
			AddWebTarget();
		});

		m_ui.btn_add->setMenu(menu);
	}
	
	connect(m_ui.btn_remove_target, SIGNAL(clicked()), this, SLOT(btn_remove_target_Click()));
	connect(m_ui.btn_edit_target, SIGNAL(clicked()), this, SLOT(btn_edit_target_Click()));
	connect(m_ui.lst_targets, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ListTargetsContextMenu(const QPoint&)));
	connect(m_ui.lst_targets, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(ListTargetsItemDoubleClick(QListWidgetItem*)));

	connect(m_ui.actionClearConsole, SIGNAL(triggered()), this, SLOT(OnClearConsole()));

	connect(m_ui.tabWidgetEditor, SIGNAL(tabCloseRequested(int)), this, SLOT(CloseTab(int)));

	QString reg_path = "HKEY_CURRENT_USER\\Software\\GameDev";
	QSettings reg(reg_path, QSettings::NativeFormat);
	QString path = reg.value("cur_path").toString();
	if (QFile::exists(path + "\\project.json"))
	{
		cur_path = path;
		update_cur_path();
	}

	LoadRecentFiles();
	LoadRecentProjects();
}

MainWindow::~MainWindow()
{


}

void MainWindow::OnAppStateChange(Qt::ApplicationState state)
{
	if (state == Qt::ApplicationInactive)
	{
		in_background = true;
	}
	else if (state == Qt::ApplicationActive)
	{
		if (in_background && !no_update)
		{
			update_cur_path();
		}
		in_background = false;
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (m_ui.tabWidgetEditor->count() > 0)
	{
		event->ignore();
		RecursiveClose();
		return;
	}
	QMainWindow::closeEvent(event);
}

void MainWindow::CommandNewProject()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("New Project"), "project.json", tr("Project(project.json)"));
	if (filename.isNull()) return;

	cur_path = QFileInfo(filename).absolutePath();
	create_default_project();

	change_path(cur_path);
}

void MainWindow::CommandOpenProject()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Project"), cur_path+"\\project.json", tr("Project(project.json)"));
	if (filename.isNull()) return;

	cur_path = QFileInfo(filename).absolutePath();
	change_path(cur_path);

}

void MainWindow::CommandCloseProject()
{
	change_path("");
}

void MainWindow::NewFile(QString path_dir)
{
	DlgNewFile dialog(this);
	if (dialog.exec() == 0) return;

	QString path = path_dir + "\\" + dialog.filename;
	if (!QFile::exists(path))
	{
		QFile file;
		file.setFileName(path);
		file.open(QFile::WriteOnly);
		if (dialog.filetype == "js")
		{
			// empty
		}
		else if (dialog.filetype == "xml")
		{
			file.write("<?xml version=\"1.0\" ?>\n<document>\n\t<scene>\n\t</scene>\n</document>\n");
		}
		else if (dialog.filetype == "json")
		{
			file.write("{}");
		}
		else if (dialog.filetype == "html")
		{
			file.write("<!DOCTYPE html>\n<html>\n\t<head>\n\t\t<meta charset=\"UTF-8\">\n\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\t</head>\n\t<body>\n\t</body>\n</html>\n");
		}
		file.close();
		update_cur_path();
	}

	OpenFile(path);

}

void MainWindow::CommandNewFile()
{
	NewFile(cur_path);
}

void MainWindow::CommandOpenFile()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), cur_path, tr("All Supported(*.js *.json *.html *.xml);;JavaScript(*.js);;JSON(*.json);;HTML(*.html);;XML(*.xml)"));
	if (filename.isNull()) return;
	OpenFile(filename);
}

void MainWindow::CommandCloseFile()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		CloseTab(idx);
	}
}

void MainWindow::CommandSave()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		SaveTab(idx);
	}
}

void MainWindow::CommandSaveAs()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		SaveAsTab(idx);
	}
}

void MainWindow::CommandSaveAll()
{
	for (int i = 0; i < m_ui.tabWidgetEditor->count(); i++)
	{
		SaveTab(i);
	}
}

void MainWindow::CommandReopen()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		ReopenTab(idx);
	}
}

void MainWindow::CommandReopenAll()
{
	for (int i = 0; i < m_ui.tabWidgetEditor->count(); i++)
	{
		ReopenTab(i);
	}
}

void MainWindow::CommandExit()
{
	close();
}

void MainWindow::CommandUndo()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->undo();
		}		
	}
}

void MainWindow::CommandRedo()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->redo();
		}
	}
}

void MainWindow::CommandComment()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->comment();
		}
	}
}

void MainWindow::CommandUpper()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->upper();
		}
	}
}

void MainWindow::CommandLower()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->lower();
		}
	}
}

void MainWindow::CommandFind()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->find();
		}
	}
}

void MainWindow::CommandFindNext()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->findnext();
		}
	}
}

void MainWindow::CommandFindPrev()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->findprev();
		}
	}
}

void MainWindow::CommandReplace()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->replace();
		}
	}
}

void MainWindow::CommandGoto()
{
	int idx = m_ui.tabWidgetEditor->currentIndex();
	if (idx >= 0)
	{
		Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
		if (editor != nullptr)
		{
			editor->gotoline();
		}
	}
}

void MainWindow::CommandProjectSettings()
{
	if (!project.data.isObject()) return;
	DlgProjectSettings dlg(this, project.data.object());
	if (dlg.exec() != 0)
	{
		project.data.setObject(dlg.projectData);
		project.Save();
		update_cur_path();
	}
}

void MainWindow::CommandHelp()
{
	QString local_path = QCoreApplication::applicationDirPath();
	QString help_path = local_path + "/help/index.html";

	HelpPage* help_page;
	if (opened_tabs.contains(help_path))
	{
		help_page = static_cast<HelpPage*>(opened_tabs[help_path]);
		m_ui.tabWidgetEditor->setCurrentWidget(help_page);
		help_page->Goto(help_path);
	}
	else
	{
		help_page = new HelpPage(m_ui.tabWidgetEditor, help_path);
		AddTabItem(help_page, help_path, "Help");
	}
}

void MainWindow::CommandAPIDoc()
{
	QString local_path = QCoreApplication::applicationDirPath();
	QString help_path = local_path + "/help/api/index.html";
	QString key_path = local_path + "/help/index.html";
	
	if (opened_tabs.contains(key_path))
	{
		HelpPage*  help_page = static_cast<HelpPage*>(opened_tabs[key_path]);
		m_ui.tabWidgetEditor->setCurrentWidget(help_page);
		help_page->Goto(help_path);
	}
	else
	{
		HelpPage* help_page = new HelpPage(m_ui.tabWidgetEditor, help_path);
		AddTabItem(help_page, key_path, "Help");
	}
}

void MainWindow::CommandTutorials()
{
	QString local_path = QCoreApplication::applicationDirPath();
	QString help_path = local_path + "/help/tutorials.html";
	QString key_path = local_path + "/help/index.html";

	if (opened_tabs.contains(key_path))
	{
		HelpPage* help_page = static_cast<HelpPage*>(opened_tabs[key_path]);
		m_ui.tabWidgetEditor->setCurrentWidget(help_page);
		help_page->Goto(help_path);
	}
	else
	{
		HelpPage* help_page = new HelpPage(m_ui.tabWidgetEditor, help_path);
		AddTabItem(help_page, key_path, "Help");
	}
}

void MainWindow::OpenJavaScript(QString file_path)
{	
	if (opened_tabs.contains(file_path))
	{		
		JSEditor* editor = static_cast<JSEditor*>(opened_tabs[file_path]);
		m_ui.tabWidgetEditor->setCurrentWidget(editor);
		editor->doc_refresh();
	}
	else
	{
		JSEditor* editor = new JSEditor(m_ui.tabWidgetEditor, file_path);
		QString filename = QFileInfo(file_path).fileName();
		AddTabItem(editor, file_path, filename);
	}

}

void MainWindow::OpenJSON(QString file_path)
{
	if (opened_tabs.contains(file_path))
	{
		JsonEditor* editor = static_cast<JsonEditor*>(opened_tabs[file_path]);
		m_ui.tabWidgetEditor->setCurrentWidget(editor);
		editor->doc_refresh();
	}
	else
	{
		JsonEditor* editor = new JsonEditor(m_ui.tabWidgetEditor, file_path);
		QString filename = QFileInfo(file_path).fileName();
		AddTabItem(editor, file_path, filename);
	}
}

void MainWindow::OpenHTML(QString file_path)
{
	if (opened_tabs.contains(file_path))
	{
		HtmlEditor* editor = static_cast<HtmlEditor*>(opened_tabs[file_path]);
		m_ui.tabWidgetEditor->setCurrentWidget(editor);
		editor->doc_refresh();
	}
	else
	{
		HtmlEditor* editor = new HtmlEditor(m_ui.tabWidgetEditor, file_path);
		QString filename = QFileInfo(file_path).fileName();
		AddTabItem(editor, file_path, filename);
	}

}

void MainWindow::OpenXML(QString file_path)
{
	if (opened_tabs.contains(file_path))
	{
		XMLEditor* editor = static_cast<XMLEditor*>(opened_tabs[file_path]);
		m_ui.tabWidgetEditor->setCurrentWidget(editor);
		editor->doc_refresh();
	}
	else
	{
		XMLEditor* editor = new XMLEditor(m_ui.tabWidgetEditor, file_path, cur_path);
		connect(editor, SIGNAL(output(QString)), this, SLOT(console_std(QString)));
		connect(editor, SIGNAL(error(QString)), this, SLOT(console_err(QString)));
		QString filename = QFileInfo(file_path).fileName();
		AddTabItem(editor, file_path, filename);
	}

}

void MainWindow::OpenFile(QString file_path)
{
	if (!QFile::exists(file_path)) return;
	QString ext = QFileInfo(file_path).suffix().toLower();
	if (ext == "js")
	{
		OpenJavaScript(file_path);
	}
	else if (ext == "json")
	{
		OpenJSON(file_path);
	}
	else if (ext == "html")
	{
		OpenHTML(file_path);
	}
	else if (ext == "xml")
	{
		OpenXML(file_path);
	}
	else
	{
		return;
	}
	AddRecentFile(file_path);
}

void MainWindow::NewDirectory(QString path_dir)
{
	DlgNewDir dialog(this);
	if (dialog.exec() == 0) return;

	QDir dir(path_dir);
	if (!dir.exists(dialog.filename))
	{
		dir.mkdir(dialog.filename);
		update_cur_path();
	}
}

void MainWindow::AddTarget()
{
	DlgEditTarget dlg(this, cur_path);
	if (dlg.exec() != 0)
	{
		QJsonObject obj_proj = project.data.object();
		QJsonArray jTargets;
		if (obj_proj.contains("targets"))
		{
			jTargets = obj_proj["targets"].toArray();
		}
		jTargets.append(dlg.jTarget);
		obj_proj["targets"] = jTargets;
		project.data.setObject(obj_proj);
		project.Save();

		QString path_in = cur_path + "\\" + dlg.jTarget["input"].toString();

		if (!QFile::exists(path_in))
		{
			QFile file;
			file.setFileName(path_in);
			file.open(QFile::WriteOnly);		
			file.close();
		}

		update_cur_path();
		m_ui.lst_targets->setCurrentRow(m_ui.lst_targets->count() - 1);

		OpenJavaScript(path_in);
	}
}

void MainWindow::AddWebTarget()
{
	DlgEditWebTarget dlg(this, cur_path);
	if (dlg.exec() != 0)
	{
		QJsonObject obj_proj = project.data.object();
		QJsonArray jTargets;
		if (obj_proj.contains("targets"))
		{
			jTargets = obj_proj["targets"].toArray();
		}
		jTargets.append(dlg.jTarget);
		obj_proj["targets"] = jTargets;
		project.data.setObject(obj_proj);
		project.Save();

		QString path_in = cur_path + "\\" + dlg.jTarget["input"].toString();

		if (!QFile::exists(path_in))
		{
			QFile file;
			file.setFileName(path_in);
			file.open(QFile::WriteOnly);
			file.close();
		}

		update_cur_path();
		m_ui.lst_targets->setCurrentRow(m_ui.lst_targets->count() - 1);

		OpenHTML(path_in);
	}
}


void MainWindow::RemoveTarget(int idx)
{
	QJsonObject obj_proj = project.data.object();
	QJsonArray jTargets = obj_proj["targets"].toArray();
	QJsonObject jTarget = jTargets[idx].toObject();
	if (QMessageBox::question(this, tr("Remove Target"), tr("Remove target\"") + jTarget["name"].toString() + "\"?")==QMessageBox::Yes)
	{
		jTargets.removeAt(idx);
		obj_proj["targets"] = jTargets;
		project.data.setObject(obj_proj);
		project.Save();
		update_cur_path();
	}
}

void MainWindow::EditTarget(int idx)
{
	QJsonObject obj_proj = project.data.object();
	QJsonArray jTargets = obj_proj["targets"].toArray();
	QJsonObject jTarget = jTargets[idx].toObject();
	
	QString fn_in = jTarget["input"].toString();
	QString ext = QFileInfo(fn_in).suffix().toLower();

	bool edited = false;
	if (ext == "js")
	{
		DlgEditTarget dlg(this, jTarget, cur_path);
		edited = (dlg.exec() != 0);
	}
	else if (ext == "html")
	{
		DlgEditWebTarget dlg(this, jTarget, cur_path);
		edited = (dlg.exec() != 0);
	}

	if (edited)
	{
		jTargets[idx] = jTarget;
		obj_proj["targets"] = jTargets;
		project.data.setObject(obj_proj);
		project.Save();
		update_cur_path();
	}

}

void MainWindow::RunTarget(int idx)
{
	QJsonObject obj_proj = project.data.object();
	QJsonArray jTargets = obj_proj["targets"].toArray();
	QJsonObject jTarget = jTargets[idx].toObject();

	QString input = jTarget["input"].toString();
	QString ext = QFileInfo(input).suffix().toLower();

	QString Location = QCoreApplication::applicationDirPath();
	bool dirty = jTarget["dirty"].toBool();
	if (dirty && ext=="js")
	{
		QDir::setCurrent(cur_path);
		
		QString output = jTarget["output"].toString();

		QFile::remove(output);

		no_update = true;
		QProcess proc;
		proc.setProgram("cmd.exe");
		QStringList args = { "/C", Location + "\\rollup.exe " + input + " --file " + output };
		proc.setArguments(args);
		proc.start();
		proc.waitForFinished();

		m_ui.consoleTextEdit->setTextColor(QColor::fromRgbF(0.0f, 0.0, 0.0f));
		m_ui.consoleTextEdit->append("running rollup.js:");

		QString err = proc.readAllStandardError();
		QRegularExpression regex("[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]");
		err.replace(regex, "");
		m_ui.consoleTextEdit->setTextColor(QColor::fromRgbF(0.0f, 0.0, 1.0f));
		m_ui.consoleTextEdit->append(err);
		m_ui.consoleTextEdit->setTextColor(QColor::fromRgbF(0.0f, 0.0, 0.0f));

		if (!QFile::exists(output))
		{
			m_ui.consoleTextEdit->append("Bundling failed!");
			jTarget["dirty"] = true;
		}
		else
		{
			m_ui.consoleTextEdit->append("Bundling succeeded!");
			jTarget["dirty"] = false;
		}
		jTargets[idx] = jTarget;
		obj_proj["targets"] = jTargets;
		project.data.setObject(obj_proj);
		project.Save();

		QScrollBar* bar = m_ui.consoleTextEdit->verticalScrollBar();
		bar->setValue(bar->maximum());

		no_update = false;
	}

	{
		QProcess* proc = new QProcess(this);
		QStringList args = { project.filename, QString::number(idx) };
		proc->start(Location + "\\QGamePlayer.exe", args);
	}

}


void MainWindow::CreateTargetShortcut(int idx)
{
	QJsonObject obj_proj = project.data.object();
	QJsonArray jTargets = obj_proj["targets"].toArray();
	QJsonObject jTarget = jTargets[idx].toObject();

	QString Location = QCoreApplication::applicationDirPath();
	QString target_path = Location + "\\QGamePlayer.exe";
	std::vector<wchar_t> wstr_path(target_path.size() + 1, 0);
	target_path.toWCharArray(wstr_path.data());
	QString arguments = QString("\"") + project.filename + "\" \"" + QString::number(idx) + "\"";
	std::vector<wchar_t> wstr_arguments(arguments.size() + 1, 0);
	arguments.toWCharArray(wstr_arguments.data());
	QString desc = "Shortcut for Three.V8 app " + jTarget["name"].toString();
	std::vector<wchar_t> wstr_desc(desc.size() + 1, 0);
	desc.toWCharArray(wstr_desc.data());

	static wchar_t wstr_desktop[MAX_PATH + 1];
	SHGetSpecialFolderPath(HWND_DESKTOP, wstr_desktop, CSIDL_DESKTOP, FALSE);
	QString shortcutAddress = QString::fromWCharArray(wstr_desktop)+ "\\" + jTarget["name"].toString() + ".lnk";
	std::vector<wchar_t> wstr_shortcutAddress(shortcutAddress.size() + 1, 0);
	shortcutAddress.toWCharArray(wstr_shortcutAddress.data());

	CComPtr<IShellLink> ipShellLink;
	CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&ipShellLink);
	CComQIPtr<IPersistFile> ipPersistFile(ipShellLink);
	ipShellLink->SetPath(wstr_path.data());
	ipShellLink->SetArguments(wstr_arguments.data());
	ipShellLink->SetDescription(wstr_desc.data());
	ipPersistFile->Save(wstr_shortcutAddress.data(), TRUE);

	QMessageBox::information(this, tr("Created shortcut"), tr("Created shortcut for target") + " \"" + jTarget["name"].toString() + "\"");
}

void MainWindow::AddTabItem(QWidget* item, QString filepath, QString name)
{
	m_ui.tabWidgetEditor->addTab(item, name);
	int idx = m_ui.tabWidgetEditor->indexOf(item);
	m_ui.tabWidgetEditor->setTabToolTip(idx, filepath);
	opened_tabs[filepath] = item;
	m_ui.tabWidgetEditor->setCurrentWidget(item);
}

void MainWindow::TrashDir(QString path)
{
	QDir dir(path);
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	QStringList list_dirs = dir.entryList();

	for (size_t i = 0; i < list_dirs.size(); i++)
	{
		QString dir_name = list_dirs[i];
		QString dir = QFileInfo(path + "\\" + dir_name).absoluteFilePath();
		TrashDir(dir);		
	}

	dir.setFilter(QDir::Files);
	QStringList list_files = dir.entryList();

	for (size_t i = 0; i < list_files.size(); i++)
	{
		QString file_name = list_files[i];
		QString file = QFileInfo(path + "\\" + file_name).absoluteFilePath();
		QFile::moveToTrash(file);
	}
	dir.removeRecursively();
}

void MainWindow::TreeFilesContextMenu(const QPoint& pos)
{
	QTreeWidgetItem* item = m_ui.tree_files->itemAt(pos);
	if (!item)
	{
		QMenu menu(tr("Root Directory Operations"), this);

		QAction action_new_file(tr("&New File"));
		connect(&action_new_file, &QAction::triggered, [this]() {
			NewFile(cur_path);
		});
		menu.addAction(&action_new_file);

		QAction action_new_directory(tr("&New Directory"));
		connect(&action_new_directory, &QAction::triggered, [this]() {
			NewDirectory(cur_path);
		});
		menu.addAction(&action_new_directory);

		menu.exec(m_ui.tree_files->viewport()->mapToGlobal(pos));

		return;
	}	
	QString path = item->toolTip(0);
	QFileInfo info(path);	
	if (info.isDir())
	{
		QMenu menu(tr("Directory Operations"), this);

		QAction action_new_file(tr("&New File"));
		connect(&action_new_file, &QAction::triggered, [this, path]() {
			NewFile(path);
		});
		menu.addAction(&action_new_file);

		QAction action_new_directory(tr("&New Directory"));
		connect(&action_new_directory, &QAction::triggered, [this, path]() {
			NewDirectory(path);
		});
		menu.addAction(&action_new_directory);

		QAction action_delete(tr("&Delete"));
		connect(&action_delete, &QAction::triggered, [this, path]() {
			if (QMessageBox::question(this, tr("Delete Directory"), tr("Delete directory") + "\"" + path + "\"?") == QMessageBox::Yes)
			{
				TrashDir(path);				
				update_cur_path();
			}
		});
		menu.addAction(&action_delete);

		menu.exec(m_ui.tree_files->viewport()->mapToGlobal(pos));
		
	}
	else if (info.isFile())
	{
		QMenu menu(tr("File Operations"), this);

		QAction action_open(tr("&Open"));
		connect(&action_open, &QAction::triggered, [this, path]() {
			OpenFile(path);
		});
		menu.addAction(&action_open);

		QAction action_delete(tr("&Delete"));
		connect(&action_delete, &QAction::triggered, [this, path]() {
			if (QMessageBox::question(this, tr("Delete File"), tr("Delete file") + "\"" + path + "\"?") == QMessageBox::Yes)
			{
				QFile::moveToTrash(path);
				update_cur_path();
			}

		});
		menu.addAction(&action_delete);

		menu.exec(m_ui.tree_files->viewport()->mapToGlobal(pos));
	}
}

void MainWindow::TreeFilesItemDoubleClick(QTreeWidgetItem* item, int column)
{
	QString path = item->toolTip(0);
	QFileInfo info(path);
	if (info.isFile())
	{
		OpenFile(path);		
	}
}

void MainWindow::ListTargetsContextMenu(const QPoint& pos)
{
	QListWidgetItem* item = m_ui.lst_targets->itemAt(pos);
	if (!item)
	{
		if (!project.data.isObject()) return;

		QMenu menu(tr("Target List Operations"), this);

		QAction action_add_target(tr("Add Target"));
		connect(&action_add_target, &QAction::triggered, [this]() {
			AddTarget();
		});
		menu.addAction(&action_add_target);

		QAction action_add_web_target(tr("Add Web Target"));
		connect(&action_add_web_target, &QAction::triggered, [this]() {
			AddWebTarget();
		});
		menu.addAction(&action_add_web_target);

		menu.exec(m_ui.lst_targets->viewport()->mapToGlobal(pos));

		return;
	}

	int idx = m_ui.lst_targets->row(item);
	QMenu menu(tr("Target Operations"), this);

	QAction action_run(tr("&Run"));
	connect(&action_run, &QAction::triggered, [this, idx]() {
		RunTarget(idx);
	});
	menu.addAction(&action_run);

	QAction action_create_shortcut(tr("Create &shortcut"));
	connect(&action_create_shortcut, &QAction::triggered, [this, idx]() {
		CreateTargetShortcut(idx);
	});
	menu.addAction(&action_create_shortcut);

	QAction action_edit(tr("&Edit"));
	connect(&action_edit, &QAction::triggered, [this, idx]() {
		EditTarget(idx);
	});
	menu.addAction(&action_edit);

	QAction action_remove(tr("&Remove"));
	connect(&action_remove, &QAction::triggered, [this, idx]() {
		RemoveTarget(idx);
	});
	menu.addAction(&action_remove);

	menu.exec(m_ui.tree_files->viewport()->mapToGlobal(pos));
}

void MainWindow::ListTargetsItemDoubleClick(QListWidgetItem* item)
{
	int idx = m_ui.lst_targets->row(item);
	RunTarget(idx);
}

void MainWindow::btn_remove_target_Click()
{
	if (!project.data.isObject()) return;
	int idx = m_ui.lst_targets->currentRow();	
	if (idx < 0) return;
	RemoveTarget(idx);
}

void MainWindow::btn_edit_target_Click()
{
	if (!project.data.isObject()) return;
	int idx = m_ui.lst_targets->currentRow();
	if (idx < 0) return;
	EditTarget(idx);

}

void MainWindow::OnClearConsole()
{
	m_ui.consoleTextEdit->clear();
}

void MainWindow::CloseTab(int idx)
{
	this->CloseTab(idx, [](bool closed) {});
}


void MainWindow::RecursiveClose()
{
	CloseTab(0, [this](bool closed) {
		if (closed)
		{
			if (m_ui.tabWidgetEditor->count() == 0)
			{
				close();
			}
			else
			{
				RecursiveClose();
			}
		}
	});
}

void MainWindow::console_std(QString str)
{
	m_ui.consoleTextEdit->setTextColor(QColor::fromRgbF(0.0f, 0.0, 0.0f));
	m_ui.consoleTextEdit->append(str);

	QScrollBar* bar = m_ui.consoleTextEdit->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void MainWindow::console_err(QString str)
{
	m_ui.consoleTextEdit->setTextColor(QColor::fromRgbF(1.0f, 0.0, 0.0f));
	m_ui.consoleTextEdit->append(str);

	QScrollBar* bar = m_ui.consoleTextEdit->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void MainWindow::SetDirty()
{
	if (!project.data.isObject()) return;
	QJsonObject obj_proj = project.data.object();
	if (!obj_proj.contains("targets")) return;
	QJsonArray targets = obj_proj["targets"].toArray();
	for (size_t i = 0; i < targets.size(); i++)
	{
		QJsonObject target = targets[i].toObject();
		target["dirty"] = true;
		targets[i] = target;
	}
	obj_proj["targets"] = targets;
	project.data.setObject(obj_proj);
	project.Save();
}

void MainWindow::SaveTab(int idx)
{
	Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
	if (editor != nullptr)
	{
		connect(editor, &Editor::doc_save_ret, this, [this, editor]() {
			int idx = m_ui.tabWidgetEditor->indexOf(editor);
			QString filepath = m_ui.tabWidgetEditor->tabToolTip(idx);
			QString ext = QFileInfo(filepath).suffix().toLower();
			QString filename = QFileInfo(filepath).fileName();
			if (filepath.startsWith(cur_path))
			{
				if (filename == "project.json")
				{
					update_cur_path();
				}
				if (ext == "js")
				{
					SetDirty();
				}
			}
		}, Qt::SingleShotConnection);
		editor->doc_save();
	}
}

void MainWindow::SaveAsTab(int idx)
{
	Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
	if (editor != nullptr)
	{
		connect(editor, &Editor::doc_save_as_ret, this, [this, editor](QString new_path) {
			if (new_path != "")
			{
				int idx = m_ui.tabWidgetEditor->indexOf(editor);
				QString filepath = m_ui.tabWidgetEditor->tabToolTip(idx);
				opened_tabs.remove(filepath);
				opened_tabs[new_path] = editor;
				m_ui.tabWidgetEditor->setTabToolTip(idx, new_path);
				m_ui.tabWidgetEditor->setTabText(idx, QFileInfo(new_path).fileName());
				update_cur_path();
			}

		}, Qt::SingleShotConnection);
		editor->doc_save_as();
	}
}

void MainWindow::ReopenTab(int idx)
{
	Editor* editor = dynamic_cast<Editor*>(m_ui.tabWidgetEditor->widget(idx));
	if (editor != nullptr)
	{
		editor->doc_refresh();
	}
}
