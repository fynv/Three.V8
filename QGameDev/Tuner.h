#pragma once

#include <QWidget>
#include <QJsonObject>

class GamePlayer;
class Tuner : public QWidget
{
	Q_OBJECT
public:
	Tuner(QWidget* parent, const QJsonObject& jobj);
	virtual ~Tuner();

	QJsonObject jobj;

signals:
	void update(QJsonObject tuning);


};
