#pragma once

#include <memory>
#include <QWidget>
#include <QTimer>
#include "ui_QGamePlayer.h"
#include <GamePlayer.h>

class QGamePlayer : public QWidget
{
	Q_OBJECT
public:
	QGamePlayer();
	virtual ~QGamePlayer();

private slots:
	void OnInit();
	void OnPaint(int width, int height);

	void BtnLoad_Click();

	void OnMouseDown(QMouseEvent* event);
	void OnMouseUp(QMouseEvent* event);
	void OnMouseMove(QMouseEvent* event);
	void OnWheel(QWheelEvent* event);

	void OnLongPress();

	void OnChar(int charCode);
	void OnControlKey(int code);

	void btn_rotate_Click();

private:
	void LoadScript(QString path);

	Ui_QGamePlayer m_ui;
	std::unique_ptr<GamePlayer> m_game_player;
	
	QString default_script = "../game/bundle_game.js";

	QTimer press_timer;
	int x_down, y_down;

	bool is_portrait = false;
};
