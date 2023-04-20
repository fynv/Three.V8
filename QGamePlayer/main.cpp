#include <memory>
#include "QGameplayer.h"
#include "PlayerWindow.h"
#include <utils/Utils.h>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	std::unique_ptr<QGamePlayer> wnd1;
	std::unique_ptr<PlayerWindow> wnd2;

	if (argc > 2)
	{
		int idx = atoi(argv[2]);
		wnd2 = std::unique_ptr<PlayerWindow>(new PlayerWindow(argv[1], idx));
		wnd2->show();
	}
	else if (exists_test(".\\client\\project.json"))
	{
		wnd2 = std::unique_ptr<PlayerWindow>(new PlayerWindow(".\\client\\project.json", 0));
		wnd2->show();
	}
	else
	{
		wnd1 = std::unique_ptr<QGamePlayer>(new QGamePlayer);
		wnd1->show();
	}

	return app.exec();
}
