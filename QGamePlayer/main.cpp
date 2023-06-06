#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include "QGameplayer.h"
#include "PlayerWindow.h"
#include "WebPlayerWindow.h"

int PlayGeneral(int argc, char* argv[])
{
	QApplication::setStyle("fusion");
	QApplication app(argc, argv);

	QGamePlayer wnd;
	wnd.show();

	return app.exec();
}

int PlayProject(int argc, char* argv[], const char* path_proj, int idx)
{
	QApplication::setStyle("fusion");
	QApplication app(argc, argv);

	std::unique_ptr<PlayerWindow> wnd1;
	std::unique_ptr<WebPlayerWindow> wnd2;

	QString filename = QString::fromLocal8Bit(path_proj);
	QFile file;
	file.setFileName(filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString text = file.readAll();
	file.close();

	QJsonDocument json_doc = QJsonDocument::fromJson(text.toUtf8());
	QJsonObject obj_proj = json_doc.object();
	QJsonObject obj_target = obj_proj["targets"].toArray()[idx].toObject();	
	QString input = obj_target["input"].toString();
	QString ext = QFileInfo(input).suffix().toLower();

	if (ext == "js")
	{
		wnd1 = std::unique_ptr<PlayerWindow>(new PlayerWindow(path_proj, idx));
		wnd1->show();
	}
	else if (ext == "html")
	{
		wnd2 = std::unique_ptr<WebPlayerWindow>(new WebPlayerWindow(path_proj, idx));
		wnd2->show();
	}

	return app.exec();
}

int main(int argc, char* argv[])
{
	if (argc > 2)
	{
		int idx = atoi(argv[2]);
		return PlayProject(argc, argv, argv[1], idx);
	}
	else if (QFile::exists(".\\client\\project.json"))
	{
		return PlayProject(argc, argv, ".\\client\\project.json", 0);
	}
	else
	{
		return PlayGeneral(argc, argv);
	}

}
