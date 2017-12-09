#ifndef UTILITY_JSON_H
#define UTILITY_JSON_H

// Project includes
#include "Utility/Result.h"
#include "Utility/Utility.h"

// Qt includes
#include <QJsonObject>
#include <QVariant>
#include <QVector>


namespace Utility
{
	Result	json_save	(const QJsonObject& json, QString filename);
	Result	json_load	(QJsonObject& json, QString filename);
	Result	json_read	(const QJsonObject& json, QString name, QJsonValue& json_value);

	template <typename T>
	Result json_read(const QJsonObject& json, QString name, T& value, const T& default_value)
	{
		QJsonValue json_value;
		auto result = json_read(json, name, json_value);
		value = (result.get_type() == ResultType::Ok ? json_value.toVariant().value<T>() : default_value);
		return result;
	}
}

#endif