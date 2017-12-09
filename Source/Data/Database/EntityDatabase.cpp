// Project includes
#include "Data/Database/EntityDatabase.h"
#include "Utility/JSON.h"

// Qt includes
#include <QJsonArray>
#include <QMap>


namespace LTTPMapTracker
{
	//================================================================================
	// Constants
	//================================================================================

	static const QString s_db_filename = "Data/EntityDatabase.json";



	//================================================================================
	// Loading
	//================================================================================

	Result EntityDatabase::load()
	{
		// Load data.
		QJsonObject json;
		
		auto load_result = json_load(json, s_db_filename);
		if (!load_result)
		{
			return load_result;
		}

		int version = 0;
		if (!json_read(json, "Version", version, 0))
		{
			return Result(false, "Unable to read version.");
		}

		Result result;

		// Read entities.
		QMap<QString, std::shared_ptr<Entity>> entities;

		QJsonValue jval_entities;
		result << json_read(json, "Entities", jval_entities);
		auto json_entities = jval_entities.toArray();

		for (auto jval_entity : json_entities)
		{
			auto json_entity = jval_entity.toObject();

			QString type_name;
			QString display_name;
			QString image_file;

			if (!(result << json_read(json_entity, "TypeName", type_name, QString())) ||
				!(result << json_read(json_entity, "DisplayName", display_name, QString())) ||
				!(result << json_read(json_entity, "Image", image_file, QString())))
			{
				continue;
			}

			if (entities.contains(type_name))
			{
				result << ResultEntry(ResultType::Warning, "Duplicate entity type name: " + type_name);
				continue;
			}

			QPixmap image;
			if (!image.load("Data/" + image_file))
			{
				result << ResultEntry(ResultType::Warning, "Unable to load image: " + image_file);
				continue;
			}

			auto entity = std::make_shared<Entity>();
			entity->m_type_name = type_name;
			entity->m_display_name = display_name;
			entity->m_image = image;
			entities.insert(type_name, entity);
		}

		// Store entities.
		std::copy(entities.begin(), entities.end(), std::back_inserter(m_entities));

		return result;
	}



	//================================================================================
	// Entity
	//================================================================================

	EntityCPtr EntityDatabase::get_entity(QString type_name) const
	{
		auto it = std::find_if(m_entities.begin(), m_entities.end(), [type_name] (EntityCPtr entity)
		{
			return (entity->m_type_name == type_name);
		});
		return (it != m_entities.end() ? *it : nullptr);
	}

	EntityList EntityDatabase::get_entities() const
	{
		return m_entities;
	}
}