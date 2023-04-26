#pragma once

#include <QJsonDocument>

class JsonData
{
public:
	QString filename = "";
	QJsonDocument data;

	void Clear();
	void Load();
	void Save();
};
