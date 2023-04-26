#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char* argv[])
{  
    QApplication::setStyle("fusion");
    QApplication app(argc, argv);

    MainWindow mainWnd(&app);
    mainWnd.showMaximized();

    return app.exec();
}
