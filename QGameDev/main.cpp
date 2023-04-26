#include <QStyleFactory>
#include <QProcess>
#include "WebView2Widget.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication::setStyle("fusion");
    QApplication app(argc, argv);

    LPWSTR version;
    GetAvailableCoreWebView2BrowserVersionString(nullptr, &version);
    if (version != nullptr)
    {
        CoTaskMemFree(version);
    }
    else
    {
        QString local_path = QCoreApplication::applicationDirPath();

        QProcess proc;
        proc.setProgram(local_path + "\\MicrosoftEdgeWebview2Setup.exe");
        proc.start();
        proc.waitForFinished();
    } 

    MainWindow mainWnd(&app);
    mainWnd.showMaximized();

    return app.exec();
}
