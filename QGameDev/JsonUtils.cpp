#include <QJsonDocument>
#include <QJsonArray>
#include "JsonUtils.h"

QString encodeJsonStringLiteral(const QString& value)
{
    return QString(
        QJsonDocument(
            QJsonArray() << value
        ).toJson(QJsonDocument::Compact)
    ).mid(1).chopped(1);
}

QString decodeJsonStringLiteral(const QString& json)
{
    return QJsonDocument::fromJson((QString("[")+json+"]").toUtf8()).array()[0].toString();
}

QString encodeJsonBoolLiteral(bool value)
{
    return QString(
        QJsonDocument(
            QJsonArray() << value
        ).toJson(QJsonDocument::Compact)
    ).mid(1).chopped(1);
}

bool decodeJsonBoolLiteral(const QString& json)
{
    return QJsonDocument::fromJson((QString("[") + json + "]").toUtf8()).array()[0].toBool();
}
