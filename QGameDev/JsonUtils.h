#pragma once

#include <QString>
QString encodeJsonStringLiteral(const QString& value);
QString decodeJsonStringLiteral(const QString& json);
QString encodeJsonBoolLiteral(bool value);
bool decodeJsonBoolLiteral(const QString& json);