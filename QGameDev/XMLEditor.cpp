#include <QFile>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QWindow>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMenu>
#include "XMLEditor.h"
#include "JsonUtils.h"
#include "FogTuner.h"
#include "SkyTuner.h"
#include "EnvLightTuner.h"
#include "GroupTuner.h"
#include "PlaneTuner.h"
#include "BoxTuner.h"
#include "SphereTuner.h"
#include "ModelTuner.h"
#include "DirectionalLightTuner.h"
#include "SceneTuner.h"
#include "ReflectorTuner.h"

XMLEditor::XMLEditor(QWidget* parent, QString file_path, QString resource_root)
	: Editor(parent)
	, file_path(file_path)
	, resource_root(resource_root)
{
	m_ui.setupUi(this);
	m_ui.splitter->setSizes(QList<int>({ INT_MAX, INT_MAX }));
	m_ui.glControl->SetFramerate(60.0f);

	connect(m_ui.glControl, SIGNAL(OnInit()), this, SLOT(OnInit()));
	connect(m_ui.glControl, SIGNAL(OnPaint(int, int)), this, SLOT(OnPaint(int, int)));
	connect(m_ui.glControl, SIGNAL(OnMouseDown(QMouseEvent*)), this, SLOT(OnMouseDown(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnMouseUp(QMouseEvent*)), this, SLOT(OnMouseUp(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnMouseMove(QMouseEvent*)), this, SLOT(OnMouseMove(QMouseEvent*)));
	connect(m_ui.glControl, SIGNAL(OnWheel(QWheelEvent*)), this, SLOT(OnWheel(QWheelEvent*)));
	connect(m_ui.glControl, SIGNAL(OnChar(int)), this, SLOT(OnChar(int)));
	connect(m_ui.glControl, SIGNAL(OnControlKey(int)), this, SLOT(OnControlKey(int)));

	press_timer.setSingleShot(true);
	press_timer.setInterval(std::chrono::milliseconds(500));
	connect(&press_timer, SIGNAL(timeout()), this, SLOT(OnLongPress()));

	connect(m_ui.tab, SIGNAL(currentChanged(int)), this, SLOT(tab_SelectionChanged(int)));
	connect(m_ui.btn_apply, SIGNAL(clicked()), this, SLOT(btn_apply_Click()));
	connect(m_ui.btn_picking, SIGNAL(toggled(bool)), this, SLOT(btn_picking_toggled(bool)));
	connect(m_ui.scene_graph, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(scene_graph_current_changed(QTreeWidgetItem*, QTreeWidgetItem*)));
	connect(m_ui.scene_graph, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(scene_graph_context_menu(const QPoint&)));

	connect(m_ui.btn_create_fog, SIGNAL(clicked()), this, SLOT(btn_create_fog_Click()));
	connect(m_ui.btn_create_sky, SIGNAL(clicked()), this, SLOT(btn_create_sky_Click()));
	connect(m_ui.btn_create_env_light, SIGNAL(clicked()), this, SLOT(btn_create_env_light_Click()));
	connect(m_ui.btn_create_group, SIGNAL(clicked()), this, SLOT(btn_create_group_Click()));
	connect(m_ui.btn_create_plane, SIGNAL(clicked()), this, SLOT(btn_create_plane_Click()));
	connect(m_ui.btn_create_box, SIGNAL(clicked()), this, SLOT(btn_create_box_Click()));
	connect(m_ui.btn_create_sphere, SIGNAL(clicked()), this, SLOT(btn_create_sphere_Click()));
	connect(m_ui.btn_create_model, SIGNAL(clicked()), this, SLOT(btn_create_model_Click()));
	connect(m_ui.btn_create_directional_light, SIGNAL(clicked()), this, SLOT(btn_create_directional_light_Click()));
	connect(m_ui.btn_create_reflector, SIGNAL(clicked()), this, SLOT(btn_create_reflector()));

	connect(m_ui.webView, SIGNAL(navigationCompleted()), this, SLOT(OnNavigationCompleted()));
	QString local_path = QCoreApplication::applicationDirPath();
	QString src = local_path + "/editor/xml_editor.html";
	m_ui.webView->initialize(src);

}


XMLEditor::~XMLEditor()
{
	m_ui.glControl->makeCurrent();
	m_game_player = nullptr;
}


void XMLEditor::SetText_gl(QString text)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->SendMessageToUser("setXML", text.toUtf8().data());
}

void XMLEditor::SetText(QString text)
{
	text_cache = text;
	changed_cache = false;
	SetText_code(text, [this, text]() {
		SetText_gl(text);
	});
}

bool XMLEditor::TextChanged_gl()
{
	if (m_game_player == nullptr) return false;
	m_ui.glControl->makeCurrent();
	QString result = QString::fromUtf8(m_game_player->SendMessageToUser("isModified", ""));
	return decodeJsonBoolLiteral(result);
}

QString XMLEditor::GetText_gl()
{
	if (m_game_player == nullptr) return "";
	m_ui.glControl->makeCurrent();
	bool changed = TextChanged_gl();
	if (changed)
	{
		text_cache = QString::fromUtf8(m_game_player->SendMessageToUser("getXML", ""));
		changed_cache = true;
	}
	return text_cache;
}

void XMLEditor::_doc_save(QString filename)
{
	GetText([this, filename](QString text) {
		QFile file(filename);
		file.open(QFile::WriteOnly);
		file.write(text.toUtf8());
		file.close();
		emit doc_save_ret();
	});
}

void XMLEditor::_doc_open(QString filename)
{
	QFile file(filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString text = file.readAll();
	file.close();
	SetText(text);
}

void XMLEditor::doc_save_as()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save as"), file_path, tr("Javascript(*.js)"));
	if (filename.isNull())
	{
		emit doc_save_as_ret("");
		return;
	}
	file_path = filename;
	connect(this, &XMLEditor::doc_save_ret, this, [this]() {
		emit doc_save_as_ret(file_path);
		}, Qt::SingleShotConnection);
	_doc_save(file_path);
}

void XMLEditor::doc_save()
{
	_doc_save(file_path);
}

void XMLEditor::doc_close()
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


void XMLEditor::doc_refresh()
{
	connect(this, &XMLEditor::doc_close_ret, this, [this](bool res) {
		if (res)
		{
			_doc_open(file_path);
		}
		emit doc_refresh_ret();
	}, Qt::SingleShotConnection);
	doc_close();
}

void XMLEditor::print_std(void* p_self, const char* cstr)
{
	XMLEditor* self = (XMLEditor*)p_self;
	emit self->output(QString::fromUtf8(cstr));
}

void XMLEditor::err_std(void* p_self, const char* cstr)
{
	XMLEditor* self = (XMLEditor*)p_self;
	emit self->error(QString::fromUtf8(cstr));
}

void XMLEditor::OnInit()
{
	if (m_game_player != nullptr) return;
	m_ui.glControl->makeCurrent();

	int width = m_ui.glControl->width();
	int height = m_ui.glControl->width();

	QString path = QCoreApplication::applicationFilePath();
	std::string cpath = path.toLocal8Bit().toStdString();
	m_game_player = std::unique_ptr<GamePlayer>(new GamePlayer(cpath.c_str(), width, height));
	m_game_player->SetPrintCallbacks(this, print_std, err_std);
	m_game_player->SetUserMessageCallback(this, user_message_callback);
	
	QString local_path = QCoreApplication::applicationDirPath();
	QString script_filename = local_path + "\\xmleditor\\bundle_index.js";
	m_game_player->LoadScript(resource_root.toLocal8Bit().data(), script_filename.toLocal8Bit().data());
}

void XMLEditor::OnPaint(int width, int height)
{
	if (m_game_player == nullptr) return;
	m_game_player->Draw(width, height);
	m_game_player->Idle();
}

struct MouseEventArgs
{
	int button;
	int clicks;
	int delta;
	int x;
	int y;
};

void XMLEditor::OnLongPress()
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->OnLongPress(x_down, y_down);
}

void XMLEditor::OnMouseDown(QMouseEvent* event)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();

	int button = -1;
	if (event->button() == Qt::MouseButton::LeftButton)
	{
		button = 0;
	}
	else if (event->button() == Qt::MouseButton::MiddleButton)
	{
		button = 1;
	}
	else if (event->button() == Qt::MouseButton::RightButton)
	{
		button = 2;
	}
	else if (event->button() == Qt::MouseButton::XButton1)
	{
		button = 3;
	}
	else if (event->button() == Qt::MouseButton::XButton2)
	{
		button = 4;
	}


	QWindow* win = m_ui.glControl->windowHandle();
	double ratio = win->devicePixelRatio();
	QPointF pos = event->position();

	MouseEventArgs args;
	args.button = button;
	args.clicks = 1;
	args.delta = 0;
	args.x = pos.x() * ratio;
	args.y = pos.y() * ratio;
	m_game_player->OnMouseDown(args.button, args.clicks, args.delta, args.x, args.y);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		x_down = args.x;
		y_down = args.y;
		press_timer.start();
	}
}

void XMLEditor::OnMouseUp(QMouseEvent* event)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();

	int button = -1;
	if (event->button() == Qt::MouseButton::LeftButton)
	{
		button = 0;
	}
	else if (event->button() == Qt::MouseButton::MiddleButton)
	{
		button = 1;
	}
	else if (event->button() == Qt::MouseButton::RightButton)
	{
		button = 2;
	}
	else if (event->button() == Qt::MouseButton::XButton1)
	{
		button = 3;
	}
	else if (event->button() == Qt::MouseButton::XButton2)
	{
		button = 4;
	}

	QWindow* win = m_ui.glControl->windowHandle();
	double ratio = win->devicePixelRatio();
	QPointF pos = event->position();

	MouseEventArgs args;
	args.button = button;
	args.clicks = 0;
	args.delta = 0;
	args.x = pos.x() * ratio;
	args.y = pos.y() * ratio;
	m_game_player->OnMouseUp(args.button, args.clicks, args.delta, args.x, args.y);

	if (press_timer.isActive())
	{
		press_timer.stop();
	}
}

void XMLEditor::OnMouseMove(QMouseEvent* event)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();

	int button = -1;
	if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
	{
		button = 0;
	}
	else if (event->buttons().testFlag(Qt::MouseButton::MiddleButton))
	{
		button = 1;
	}
	else if (event->buttons().testFlag(Qt::MouseButton::RightButton))
	{
		button = 2;
	}
	else if (event->buttons().testFlag(Qt::MouseButton::XButton1))
	{
		button = 3;
	}
	else if (event->buttons().testFlag(Qt::MouseButton::XButton2))
	{
		button = 4;
	}

	QWindow* win = m_ui.glControl->windowHandle();
	double ratio = win->devicePixelRatio();
	QPointF pos = event->position();

	MouseEventArgs args;
	args.button = button;
	args.clicks = 0;
	args.delta = 0;
	args.x = pos.x() * ratio;
	args.y = pos.y() * ratio;
	m_game_player->OnMouseMove(args.button, args.clicks, args.delta, args.x, args.y);

	if (press_timer.isActive())
	{
		int x = args.x;
		int y = args.y;

		int dx = x - x_down;
		int dy = y - y_down;
		double dis = sqrt((double)(dx * dx) + (double)(dy * dy));
		if (dis > 3.0)
		{
			press_timer.stop();
		}
	}
}

void XMLEditor::OnWheel(QWheelEvent* event)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();

	QWindow* win = m_ui.glControl->windowHandle();
	double ratio = win->devicePixelRatio();
	QPointF pos = event->position();

	MouseEventArgs args;
	args.button = -1;
	args.clicks = 0;
	args.delta = event->angleDelta().y();
	args.x = pos.x() * ratio;
	args.y = pos.y() * ratio;
	m_game_player->OnMouseWheel(args.button, args.clicks, args.delta, args.x, args.y);
}

void XMLEditor::OnChar(int charCode)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->OnChar(charCode);
}

void XMLEditor::OnControlKey(int code)
{
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->OnControlKey(code);
}


void XMLEditor::OnNavigationCompleted()
{
	if (m_game_player == nullptr)
	{
		OnInit();
	}
	_doc_open(file_path);
}

void XMLEditor::undo()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('undo');");
}

void XMLEditor::redo()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('redo');");
}

void XMLEditor::comment()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('togglecomment');");
}

void XMLEditor::upper()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('touppercase');");
}

void XMLEditor::lower()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('tolowercase');");
}

void XMLEditor::find()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('find');");
}

void XMLEditor::findnext()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('findnext');");
}

void XMLEditor::findprev()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('findprevious');");
}

void XMLEditor::replace()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('replace');");
}

void XMLEditor::gotoline()
{
	m_ui.webView->ExecuteScriptAsync("editor.execCommand('gotoline');");
}

void XMLEditor::tab_SelectionChanged(int idx)
{
	if (idx == cur_tab) return;

	if (cur_tab == 0)
	{
		TextChanged_code([this, idx](bool changed) {
			if (changed)
			{
				if (QMessageBox::question(this, tr("Apply changes"), tr("Code has changed, apply it to view?")) == QMessageBox::Yes)
				{
					GetText_code([this, idx](QString text) {
						SetText_gl(text);
						cur_tab = idx;
					});
				}
				else
				{
					cur_tab = idx;
				}
			}
			else
			{
				cur_tab = idx;
			}
		});
	}
	else
	{
		if (cur_tab == 1)
		{
			m_ui.btn_picking->setChecked(false);
		}
		if (idx == 0)
		{
			if (TextChanged_gl())
			{
				GetText_gl();
			}
			SetText_code(text_cache, [this, idx]() {
				cur_tab = idx;
			});
		}
		else
		{
			cur_tab = idx;
		}

	}

}

void XMLEditor::btn_apply_Click()
{
	GetText_code([this](QString text) {
		SetText_gl(text);
	});
}

void XMLEditor::btn_picking_toggled(bool checked)
{
	m_ui.scene_graph->setEnabled(!checked);
	if (m_game_player == nullptr) return;
	m_ui.glControl->makeCurrent();
	m_game_player->SendMessageToUser("picking", checked?"on":"off");
}

void XMLEditor::update_index_item(QTreeWidgetItem* item, const QJsonObject& obj)
{
	QJsonObject dict = index["index"].toObject();
	QJsonArray children = obj["children"].toArray();	
	for (size_t i = 0; i < children.size(); i++)
	{
		QString key = children[i].toString();				
		QJsonObject child = dict[key].toObject();
		QString tagName = child["tagName"].toString();
		QJsonObject attributes = child["attributes"].toObject();
		QString name;
		if (attributes.contains("name"))
		{
			name = attributes["name"].toString();
		}
		else
		{
			name = tagName;
		}		

		QString icon_name = "object3d.png";
		QString tooltip = "General 3D Object";
		if (tagName == "camera")
		{
			icon_name = "camera.png";
			tooltip = tr("A Camera");
		}
		else if (tagName == "fog")
		{
			icon_name = "fog.png";
			tooltip = tr("Fog settings");
		}
		else if (tagName == "sky")
		{
			icon_name = "sky.png";
			tooltip = tr("Sky settings");
		}
		else if (tagName == "env_light")
		{
			icon_name = "env_light.png";
			tooltip = tr("Environment light settings");
		}
		else if (tagName == "control")
		{
			icon_name = "control.png";
			tooltip = tr("Control settings");
		}
		else if (tagName == "group")
		{
			icon_name = "group.png";
			tooltip = tr("Group of 3D Objects");
		}
		else if (tagName == "plane")
		{
			icon_name = "plane.png";
			tooltip = tr("Rectangle Plane");
		}
		else if (tagName == "box")
		{
			icon_name = "box.png";
			tooltip = tr("3D Box Shape");
		}
		else if (tagName == "sphere")
		{
			icon_name = "sphere.png";
			tooltip = tr("3D Sphere Shape");
		}
		else if (tagName == "model")
		{
			icon_name = "model.png";
			tooltip = tr("3D Model");
		}
		else if (tagName == "avatar")
		{
			icon_name = "avatar.png";
			tooltip = tr("3D Avatar");
		}
		else if (tagName == "directional_light")
		{
			icon_name = "directional_light.png";
			tooltip = tr("Directional light");
		}
		else if (tagName == "reflector")
		{
			icon_name = "reflector.png";
			tooltip = tr("Reflector");
		}

		QTreeWidgetItem* subitem = new QTreeWidgetItem(item);
		subitem->setText(0, name);
		subitem->setIcon(0, QIcon(QString(":/images/")+ icon_name));
		subitem->setToolTip(0, tooltip);
		subitem->setData(0, Qt::UserRole, key);
		TreeItemMap[key] = subitem;
		update_index_item(subitem, child);
	}
	item->setExpanded(true);
}

void XMLEditor::update_index()
{
	m_ui.scene_graph->clear();
	TreeItemMap.clear();

	QString key = index["root"].toString();
	QJsonObject dict = index["index"].toObject();
	QJsonObject root = dict[key].toObject();
	QString tagName = root["tagName"].toString();
	QJsonObject attributes = root["attributes"].toObject();
	QString name;
	if (attributes.contains("name"))
	{
		name = attributes["name"].toString();
	}
	else
	{
		name = tagName;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem(m_ui.scene_graph->invisibleRootItem());
	item->setText(0, name);
	item->setIcon(0, QIcon(":/images/scene.png"));
	item->setToolTip(0, tr("Root item of the scene"));
	item->setData(0, Qt::UserRole, key);
	TreeItemMap[key] = item;
	update_index_item(item, root);
}

std::string XMLEditor::index_loaded(const char* json_str)
{	
	index = QJsonDocument::fromJson(json_str).object();
	update_index();
	return "";
}

void XMLEditor::tuner_update(QJsonObject tuning)
{
	{
		QJsonObject key_map = index["index"].toObject();
		key_map[picked_key] = tuner->jobj;
		index["index"] = key_map;
	}

	m_ui.glControl->makeCurrent();
	std::string res = m_game_player->SendMessageToUser("tuning", QJsonDocument(tuning).toJson());
	if (res!="")
	{
		tuner->update_result(QJsonDocument::fromJson(res.c_str()).object());	
		QJsonObject key_map = index["index"].toObject();
		key_map[picked_key] = tuner->jobj;
		index["index"] = key_map;
	}

	if (tuning.contains("name"))
	{
		QTreeWidgetItem* item = TreeItemMap[picked_key];
		item->setText(0, tuning["name"].toString());
	}
}

void XMLEditor::tuner_generate(QJsonObject tuning)
{
	QJsonObject key_map = index["index"].toObject();
	key_map[picked_key] = tuner->jobj;
	index["index"] = key_map;

	m_ui.glControl->makeCurrent();
	m_game_player->SendMessageToUser("generate", QJsonDocument(tuning).toJson());

}

void XMLEditor::tuner_initialize()
{
	m_ui.glControl->makeCurrent();
	std::string res = m_game_player->SendMessageToUser("initialize", "{}");
	if (res != "")
	{
		EnvLightTuner* env_tuner = dynamic_cast<EnvLightTuner*>(tuner);
		if (env_tuner != nullptr)
		{
			env_tuner->initialize_result(QString::fromUtf8(res));
		}
	}
}

void XMLEditor::add_prim_ref()
{
	m_ui.glControl->makeCurrent();
	m_game_player->SendMessageToUser("add_prim_ref", "");
}

std::string XMLEditor::object_picked(const char* key)
{
	picked_key = key;
	if (tuner != nullptr)
	{
		m_ui.property_area->removeWidget(tuner);
		tuner->deleteLater();
		tuner = nullptr;
	}

	static QSet<QString> tags3d = { "scene", "group", "plane", "box", "sphere", "model", "avatar", "directional_light", "reflector"};

	m_ui.grp_scene_objs->setEnabled(false);
	m_ui.grp_3d_objs->setEnabled(false);
	if (picked_key != "")
	{
		QJsonObject picked_obj = index["index"].toObject()[picked_key].toObject();
		QString tag = picked_obj["tagName"].toString();
		if (tags3d.contains(tag))
		{
			m_ui.grp_3d_objs->setEnabled(true);
		}
		if (tag == "scene")
		{
			m_ui.grp_scene_objs->setEnabled(true);
			tuner = new SceneTuner(m_ui.property_area, picked_obj);
			connect(tuner, SIGNAL(generate(QJsonObject)), this, SLOT(tuner_generate(QJsonObject)));
		}
		else if (tag == "fog")
		{			
			tuner = new FogTuner(m_ui.property_area, picked_obj);
		}
		else if (tag == "sky")
		{
			tuner = new SkyTuner(m_ui.property_area, picked_obj);
		}
		else if (tag == "env_light")
		{
			tuner = new EnvLightTuner(m_ui.property_area, picked_obj);
			connect(tuner, SIGNAL(generate(QJsonObject)), this, SLOT(tuner_generate(QJsonObject)));
			connect(tuner, SIGNAL(initialize()), this, SLOT(tuner_initialize()));
		}
		else if (tag == "group")
		{
			tuner = new GroupTuner(m_ui.property_area, picked_obj);
		}
		else if (tag == "plane")
		{
			tuner = new PlaneTuner(m_ui.property_area, picked_obj);
		}
		else if (tag == "box")
		{
			tuner = new BoxTuner(m_ui.property_area, picked_obj);
		}
		else if (tag == "sphere")
		{
			tuner = new SphereTuner(m_ui.property_area, picked_obj);
		}
		else if (tag == "model" || tag == "avatar")
		{
			tuner = new ModelTuner(m_ui.property_area, picked_obj, tag == "avatar");
		}
		else if (tag == "directional_light")
		{
			tuner = new DirectionalLightTuner(m_ui.property_area, picked_obj);
		}
		else if (tag == "reflector")
		{
			tuner = new ReflectorTuner(m_ui.property_area, picked_obj);
			connect(tuner, SIGNAL(add_ref_prim()), this, SLOT(add_prim_ref()));
		}

		if (tuner != nullptr)
		{
			m_ui.property_area->addWidget(tuner);
			connect(tuner, SIGNAL(update(QJsonObject)), this, SLOT(tuner_update(QJsonObject)));
		}

		QTreeWidgetItem* treeItem = TreeItemMap[picked_key];
		m_ui.scene_graph->setCurrentItem(treeItem);
	}
	m_ui.btn_picking->setChecked(false);
	return "";
}

std::string XMLEditor::object_created(const char* json_str)
{
	QJsonObject key_map = index["index"].toObject();
	QJsonObject info = QJsonDocument::fromJson(json_str).object();
	for (size_t i = 0; i < info.size(); i++)
	{
		QString key = info.keys()[i];
		QJsonObject node = info[key].toObject();
		QString key_parent = node["parent"].toString();
		QJsonObject parent = key_map[key_parent].toObject();
		key_map[key] = node;
		QJsonArray children = parent["children"].toArray();
		children.append(key);
		parent["children"] = children;
		key_map[key_parent] = parent;
	}
	index["index"] = key_map;
	update_index();
	return "";
}

std::string XMLEditor::object_removed(const char* key)
{
	QJsonObject key_map = index["index"].toObject();
	QJsonObject node = key_map[key].toObject();
	key_map.remove(key);

	QString key_parent = node["parent"].toString();
	QJsonObject parent = key_map[key_parent].toObject();
	QJsonArray children = parent["children"].toArray();

	for (size_t i = 0; i < children.size(); i++)
	{
		QString child_key = children[i].toString();
		if (child_key == key)
		{
			children.removeAt(i);
			break;
		}
	}
	parent["children"] = children;
	key_map[key_parent] = parent;
	index["index"] = key_map;
	update_index();

	return "";
}

void XMLEditor::prim_ref_picked(const char* json_prim_ref)
{
	ReflectorTuner* refl_tuner = dynamic_cast<ReflectorTuner*>(tuner);
	if (refl_tuner != nullptr)
	{
		refl_tuner->prim_ref_picked(json_prim_ref);
	}
}

std::string XMLEditor::user_message_callback(void* ptr, const char* name, const char* msg)
{
	XMLEditor* self = (XMLEditor*)ptr;
	std::string s_name = name;
	if (s_name == "index_loaded")
	{
		return self->index_loaded(msg);
	}
	else if (s_name == "object_picked")
	{
		return self->object_picked(msg);
	}
	else if (s_name == "object_created")
	{
		return self->object_created(msg);
	}
	else if (s_name == "object_removed")
	{
		return self->object_removed(msg);
	}
	else if (s_name == "add_ref_prim")
	{
		self->prim_ref_picked(msg);
	}
	return "";
}


void XMLEditor::scene_graph_current_changed(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (m_ui.btn_picking->isChecked()) return;
	if (current != nullptr)
	{
		QString key = current->data(0, Qt::UserRole).toString();
		if (m_game_player == nullptr) return;
		m_ui.glControl->makeCurrent();
		m_game_player->SendMessageToUser("pick_obj", key.toUtf8().data());
	}
}

void XMLEditor::scene_graph_context_menu(const QPoint& pos)
{
	QTreeWidgetItem* item = m_ui.scene_graph->itemAt(pos);
	if (item == nullptr) return;

	QString key = item->data(0, Qt::UserRole).toString();
	if (key == index["root"].toString()) return;

	QJsonObject obj = index["index"].toObject()[key].toObject();
	QString tag = obj["tagName"].toString();
	QJsonObject att = obj["attributes"].toObject();
	QString name;
	if (att.contains("name"))
	{
		name = att["name"].toString();
	}
	else
	{
		name = tag;
	}

	QMenu menu(tr("Item Operations"), this);
	QAction action_remove(tr("Remove"));
	connect(&action_remove, &QAction::triggered, [this,key,tag, name]() {
		if (QMessageBox::question(this, tr("Remove Item"), tr("Remove ") + tag + tr(" object") + "\"" + name + tr("\" and all its children from scene?")) == QMessageBox::Yes)
		{
			m_ui.glControl->makeCurrent();
			m_game_player->SendMessageToUser("remove", key.toUtf8().data());
		}		
	});
	menu.addAction(&action_remove);

	menu.exec(m_ui.scene_graph->viewport()->mapToGlobal(pos));

}

void XMLEditor::req_create_scene_obj(QString tag)
{
	QJsonObject key_map = index["index"].toObject();
	QJsonObject base_obj = key_map[picked_key].toObject();
	QJsonArray children = base_obj["children"].toArray();

	QString key_existing = "";
	for (int i = 0; i < children.size(); i++)
	{
		QString key = children[i].toString();
		QJsonObject child = key_map[key].toObject();
		if (child["tagName"].toString() == tag)
		{
			key_existing = key;
			break;
		}
	}

	if (key_existing != "")
	{
		m_ui.glControl->makeCurrent();
		m_game_player->SendMessageToUser("pick_obj", key_existing.toUtf8().data());
		return;
	}

	QJsonObject req;
	req["base_key"] = picked_key;
	req["tag"] = tag;	

	m_ui.glControl->makeCurrent();
	m_game_player->SendMessageToUser("create", QJsonDocument(req).toJson().data());
}

void XMLEditor::req_create_obj3d(QString tag)
{
	QJsonObject req;
	req["base_key"] = picked_key;
	req["tag"] = tag;

	m_ui.glControl->makeCurrent();
	m_game_player->SendMessageToUser("create", QJsonDocument(req).toJson().data());
}

void XMLEditor::btn_create_fog_Click()
{
	req_create_scene_obj("fog");
}

void XMLEditor::btn_create_sky_Click()
{
	req_create_scene_obj("sky");
}

void XMLEditor::btn_create_env_light_Click()
{
	req_create_scene_obj("env_light");
}

void XMLEditor::btn_create_group_Click()
{
	req_create_obj3d("group");
}

void XMLEditor::btn_create_plane_Click()
{
	req_create_obj3d("plane");
}

void XMLEditor::btn_create_box_Click()
{
	req_create_obj3d("box");
}

void XMLEditor::btn_create_sphere_Click()
{
	req_create_obj3d("sphere");
}

void XMLEditor::btn_create_model_Click()
{
	req_create_obj3d("model");
}

void XMLEditor::btn_create_directional_light_Click()
{
	req_create_obj3d("directional_light");
}

void XMLEditor::btn_create_reflector()
{
	req_create_obj3d("reflector");
}
