// Project includes
#include "Utility/JSON.h"

// Qt includes
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>


namespace Utility
{
	//================================================================================
	// JSON Utility
	//================================================================================

	int get_json_line(QString data, int byte_index)
	{
		int line = 1;
		int index = 0;
		
		while (index != -1)
		{
			index = data.indexOf('\n', index + 1);
			if (index >= byte_index)
			{
				break;
			}
			++line;
		}

		return line;
	}



	//================================================================================
	// JSON
	//================================================================================

	Result json_save(const QJsonObject& json, QString filename)
	{
		QFile fh(filename);
		if (!fh.open(QIODevice::WriteOnly))
		{
			return Result(false, "Failed to open file for writing: " + filename);
		}

		fh.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
		fh.close();

		return Result();
	}

	//--------------------------------------------------------------------------------

	Result json_load(QJsonObject& json, QString filename)
	{
		QFile fh(filename);
		if (!fh.open(QIODevice::ReadOnly))
		{
			return Result(false, "Failed to open file for reading: " + filename);
		}

		QJsonParseError error;
		auto contents = fh.readAll();
		json = QJsonDocument::fromJson(contents, &error).object();

		fh.close();

		if (error.error != QJsonParseError::NoError)
		{
			return Result(false, QString("Parse error (line %1): %2").arg(get_json_line(contents, error.offset)).arg(error.errorString()));
		}

		return Result();
	}

	//--------------------------------------------------------------------------------

	Result json_read(const QJsonObject& json, QString name, QJsonValue& json_value)
	{
		auto it = json.find(name);
		bool result = (it != json.end());
		json_value = (result ? *it : QJsonValue());
		return (result ? Result() : Result(ResultType::Warning, QString("Failed to read value for property '%1'.").arg(name)));
	}
}