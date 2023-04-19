#include "QGameplayer.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QGamePlayer widget;
	widget.show();

	return app.exec();
}
