#pragma once

#include <Windows.h>
#include <WebView2.h>
#include <QWidget>

class WebView2Widget : public QWidget
{
	Q_OBJECT
public:
	WebView2Widget(QWidget* parent);
	virtual ~WebView2Widget();

	void initialize(QString directory, QString relpath);	

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	ICoreWebView2Controller* webviewController = nullptr;
	ICoreWebView2* webview = nullptr;
	QString directory;
	QString url;

};
