#include <QFile>
#include <QJsonDocument>
#include "JsonData.h"

void JsonData::Clear()
{
	filename = "";
	data = QJsonDocument();
}

void JsonData::Load()
{
	QFile file;
	file.setFileName(filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString text = file.readAll();
	file.close();
	data = QJsonDocument::fromJson(text.toUtf8());	
}

void JsonData::Save()
{
	QFile file;
	file.setFileName(filename);
	file.open(QFile::WriteOnly);
	file.write(data.toJson());
	file.close();
}

