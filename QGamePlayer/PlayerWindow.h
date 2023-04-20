#pragma once

#include <memory>
#include <QWidget>
#include <QTimer>
#include "ui_PlayerWindow.h"
#include <GamePlayer.h>

class PlayerWindow : public QWidget
{
	Q_OBJECT
public:
	PlayerWindow(const char* path_proj, int idx);
	virtual ~PlayerWindow();

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

	void OnClearConsole();

private:
	QString m_script_path;
	void LoadScript(QString path);

	Ui_PlayerWindow m_ui;
	std::unique_ptr<GamePlayer> m_game_player;

	QTimer press_timer;
	int x_down, y_down;

	static void print_std(void* p_self, const char* cstr);
	static void err_std(void* p_self, const char* cstr);


};

