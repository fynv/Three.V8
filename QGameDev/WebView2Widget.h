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

	void initialize(QString src);
	void navigate(QString src);

	template<typename TCallback>
	void ExecuteScriptAsync(QString script, TCallback callback);

	void ExecuteScriptAsync(QString script);	

signals:
	void navigationCompleted();

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	ICoreWebView2Controller* webviewController = nullptr;
	ICoreWebView2* webview = nullptr;
	QString source;

};

#include <wrl.h>

template<typename TCallback>
void WebView2Widget::ExecuteScriptAsync(QString script, TCallback callback)
{
	std::vector<wchar_t> wstr_script(script.length() + 1, 0);
	script.toWCharArray(wstr_script.data());
	this->webview->ExecuteScript(wstr_script.data(), Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
		[this, callback](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
			QString result = QString::fromWCharArray(resultObjectAsJson);
			callback(result);
			return S_OK;
		}).Get());
}
