// Project includes
#include "Data/Database/ItemDatabase.h"
#include "Data/Instance/Instance.h"
#include "Utility/JSON.h"

// Qt includes
#include <QJsonArray>
#include <QMap>


namespace LTTPMapTracker
{
	//================================================================================
	// Constants
	//================================================================================

	static const QString s_db_filename = "Data/ItemDatabase.json";



	//================================================================================
	// Loading
	//================================================================================

	Result ItemDatabase::load(const EntityDatabase& entity_db)
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

		// Read items.
		QMap<QString, std::shared_ptr<Item>> items;

		QJsonValue jval_items;
		result << json_read(json, "Items", jval_items);
		auto json_items = jval_items.toArray();

		for (auto jval_item : jval_items.toArray())
		{
			auto entity_name = jval_item.toString();

			if (items.contains(entity_name))
			{
				result << ResultEntry(ResultType::Warning, "Duplicate item type name: " + entity_name);
				continue;
			}

			auto entity = entity_db.get_entity(entity_name);
			if (entity == nullptr)
			{
				result << ResultEntry(ResultType::Warning, "Entity not found: " + entity_name);
				continue;
			}

			auto item = std::make_shared<Item>();
			item->m_entity = entity;
			items.insert(entity_name, item);
		}

		// Store items.
		std::copy(items.begin(), items.end(), std::back_inserter(m_items));

		return result;
	}



	//================================================================================
	// Data
	//================================================================================

	ItemCPtr ItemDatabase::get_item(QString type_name) const
	{
		auto it = std::find_if(m_items.begin(), m_items.end(), [type_name] (ItemCPtr item)
		{
			return (item->m_entity->m_type_name == type_name);
		});
		return (it != m_items.end() ? *it : nullptr);
	}

	ItemCPtr ItemDatabase::get_item(EntityCPtr entity) const
	{
		auto it = std::find_if(m_items.begin(), m_items.end(), [entity] (ItemCPtr item)
		{
			return (item->m_entity == entity);
		});
		return (it != m_items.end() ? *it : nullptr);
	}

	ItemList ItemDatabase::get_items() const
	{
		return m_items;
	}
}