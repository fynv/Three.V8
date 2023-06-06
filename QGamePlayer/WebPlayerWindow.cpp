#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include "WebPlayerWindow.h"

WebPlayerWindow::WebPlayerWindow(const char* path_proj, int idx)
{
	m_ui.setupUi(this);

	QString filename = QString::fromLocal8Bit(path_proj);
	QFile file;
	file.setFileName(filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString text = file.readAll();
	file.close();

	QJsonDocument json_doc = QJsonDocument::fromJson(text.toUtf8());
	QJsonObject obj_proj = json_doc.object();
	QJsonObject obj_target = obj_proj["targets"].toArray()[idx].toObject();
	QString name = obj_target["name"].toString();
	QString input = obj_target["input"].toString();

	this->setWindowTitle(name);

	QString dir_proj = QFileInfo(filename).path();
	if (obj_target.contains("width") && obj_target.contains("height"))
	{
		int width = obj_target["width"].toInt();
		int height = obj_target["height"].toInt();

		this->resize(width, height);
	}

	m_ui.webView->initialize(dir_proj, input);
}


WebPlayerWindow::~WebPlayerWindow()
{


}