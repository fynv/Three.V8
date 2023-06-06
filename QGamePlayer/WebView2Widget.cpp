#include <vector>
#include <QResizeEvent>
#include <QApplication>
#include <wrl.h>
#include "WebView2Widget.h"

using namespace Microsoft::WRL;

WebView2Widget::WebView2Widget(QWidget* parent) : QWidget(parent)
{
	this->setAttribute(Qt::WA_OpaquePaintEvent, true);
	this->setUpdatesEnabled(false);

}

WebView2Widget::~WebView2Widget()
{
	if (webview != nullptr)
	{	
		webview->Release();
	}

	if (webviewController != nullptr)
	{
		webviewController->Release();
	}

}

void WebView2Widget::initialize(QString directory, QString relpath)
{
	if (webview != nullptr)
	{
		webview->Release();
	}

	if (webviewController != nullptr)
	{
		webviewController->Release();
	}

	this->directory = directory;
	this->url = QString("https://test.site/")+relpath;
	
	HWND hWnd = (HWND)(this->winId());
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[hWnd, this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
				env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[hWnd, this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr)
						{
							controller->AddRef();
							webviewController = controller;
							webviewController->get_CoreWebView2(&webview);
							webview->AddRef();

							ICoreWebView2Settings* settings;
							webview->get_Settings(&settings);
							settings->AddRef();
							settings->put_IsScriptEnabled(TRUE);
							settings->put_AreDefaultScriptDialogsEnabled(TRUE);
							settings->put_IsWebMessageEnabled(TRUE);
							settings->Release();

							RECT bounds;
							GetClientRect(hWnd, &bounds);
							webviewController->put_Bounds(bounds);

							std::vector<wchar_t> wstr_dir(this->directory.size() + 1, 0);
							this->directory.toWCharArray(wstr_dir.data());

							std::vector<wchar_t> wstr_url(this->url.size() + 1, 0);
							this->url.toWCharArray(wstr_url.data());

							ICoreWebView2_3* webview2_3 = (ICoreWebView2_3*)webview;
							webview2_3->SetVirtualHostNameToFolderMapping(L"test.site", wstr_dir.data(), COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);							
							webview->Navigate(wstr_url.data());
						}

						return S_OK;
					}).Get()
						);
				return S_OK;
			}).Get()
		);
}

void WebView2Widget::resizeEvent(QResizeEvent* event)
{
	if (webviewController != nullptr)
	{
		HWND hWnd = (HWND)(this->winId());
		RECT bounds;
		GetClientRect(hWnd, &bounds);
		webviewController->put_Bounds(bounds);
	}
}
