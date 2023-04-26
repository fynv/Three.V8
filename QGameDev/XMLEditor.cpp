#include <QFile>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include "XMLEditor.h"
#include "JsonUtils.h"

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

	MouseEventArgs args;
	args.button = button;
	args.clicks = 1;
	args.delta = 0;
	args.x = event->x();
	args.y = event->y();
	m_game_player->OnMouseDown(args.button, args.clicks, args.delta, args.x, args.y);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		x_down = event->x();
		y_down = event->y();
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

	MouseEventArgs args;
	args.button = button;
	args.clicks = 0;
	args.delta = 0;
	args.x = event->x();
	args.y = event->y();
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

	MouseEventArgs args;
	args.button = button;
	args.clicks = 0;
	args.delta = 0;
	args.x = event->x();
	args.y = event->y();
	m_game_player->OnMouseMove(args.button, args.clicks, args.delta, args.x, args.y);

	if (press_timer.isActive())
	{
		int x = event->x();
		int y = event->y();

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

	MouseEventArgs args;
	args.button = -1;
	args.clicks = 0;
	args.delta = event->angleDelta().y();
	args.x = qRound(event->position().x());
	args.y = qRound(event->position().y());
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

