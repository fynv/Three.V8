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

void WebView2Widget::initialize(QString src)
{
	if (webview != nullptr)
	{
		webview->Release();
	}

	if (webviewController != nullptr)
	{
		webviewController->Release();
	}

	this->source = src;
	
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

							std::vector<wchar_t> wstr_src(source.size() + 1, 0);
							source.toWCharArray(wstr_src.data());
							webview->Navigate(wstr_src.data());

							EventRegistrationToken token;
							webview->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
								[this](ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args)-> HRESULT {
									emit navigationCompleted();
									return S_OK;
								}).Get(), &token);

							webviewController->add_AcceleratorKeyPressed(Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>(
								[this](ICoreWebView2Controller* sender, ICoreWebView2AcceleratorKeyPressedEventArgs* args) -> HRESULT {
									COREWEBVIEW2_KEY_EVENT_KIND kind;
									UINT key;
									args->get_KeyEventKind(&kind);
									args->get_VirtualKey(&key);
									bool ctrl = GetKeyState(VK_CONTROL) < 0;
									bool shift = GetKeyState(VK_SHIFT) < 0;
									bool alt = GetKeyState(VK_MENU) < 0;
									
									QEvent::Type eventType;
									if (kind == COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN || 
										kind == COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN)
									{
										eventType = QKeyEvent::KeyPress;
									}
									else
									{
										eventType = QKeyEvent::KeyRelease;
									}

									Qt::KeyboardModifiers modifiers;
									if (ctrl) modifiers |= Qt::ControlModifier;
									if (shift) modifiers |= Qt::ShiftModifier;
									if (alt) modifiers |= Qt::AltModifier;

									// Ctrl + N
									// Ctrl + Shift + N
									if (key == 'N' && ctrl && (!alt)) 
									{
										args->put_Handled(TRUE);
										QKeyEvent* key_press = new QKeyEvent(eventType, Qt::Key_N, modifiers);
										QApplication::sendEvent(this, key_press);
									}		

									// Ctrl + O
									// Ctrl + Shift + O
									if (key == 'O' && ctrl && (!alt)) 
									{
										args->put_Handled(TRUE);
										QKeyEvent* key_press = new QKeyEvent(eventType, Qt::Key_O, modifiers);
										QApplication::sendEvent(this, key_press);
									}		

									// Ctrl + S
									if (key == 'S' && ctrl && (!shift) && (!alt))
									{
										args->put_Handled(TRUE);
										QKeyEvent* key_press = new QKeyEvent(eventType, Qt::Key_S, modifiers);
										QApplication::sendEvent(this, key_press);
									}

									// F5
									if (key == VK_F5 && (!ctrl) && (!shift) && (!alt))
									{
										args->put_Handled(TRUE);
										QKeyEvent* key_press = new QKeyEvent(eventType, Qt::Key_F5, modifiers);
										QApplication::sendEvent(this, key_press);
									}

									// Ctrl + F1
									if (key == VK_F5 && ctrl && (!shift) && (!alt))
									{
										args->put_Handled(TRUE);
										QKeyEvent* key_press = new QKeyEvent(eventType, Qt::Key_F1, modifiers);
										QApplication::sendEvent(this, key_press);
									}

									// Alt + F7
									if (key == VK_F7 && (!ctrl) && (!shift) && alt)
									{
										args->put_Handled(TRUE);
										QKeyEvent* key_press = new QKeyEvent(eventType, Qt::Key_F7, modifiers);
										QApplication::sendEvent(this, key_press);
									}

									return S_OK;

								}).Get(), &token);
						}

						return S_OK;
					}).Get()
						);
				return S_OK;
			}).Get()
		);
}

void WebView2Widget::navigate(QString src)
{
	this->source = src;
	std::vector<wchar_t> wstr_src(this->source.size()+1, 0);
	this->source.toWCharArray(wstr_src.data());
	this->webview->Navigate(wstr_src.data());
}

void WebView2Widget::ExecuteScriptAsync(QString script)
{
	this->ExecuteScriptAsync(script, [](QString result) {});
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
