#pragma once

#include <QWidget>
#include "ui_WebPlayerWindow.h"

class WebPlayerWindow : public QWidget
{
	Q_OBJECT
public:
	WebPlayerWindow(const char* path_proj, int idx);
	virtual ~WebPlayerWindow();

private:
	Ui_WebPlayerWindow m_ui;
};

