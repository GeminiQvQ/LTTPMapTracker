// Project includes
#include "Data/Instance/InstanceData.h"
#include "Data/Instance/Instance.h"
#include "Data/Schema/Schema.h"
#include "Utility/JSON.h"
#include "Utility/Result.h"

// Qt includes
#include <QJsonArray>
#include <QJsonObject>


namespace LTTPMapTracker
{
	//================================================================================
	// Item
	//================================================================================

	InstanceItemData::InstanceItemData()
		: m_cleared(false)
		, m_accessible(false)
		, m_accessible_cached(false)
	{
	}

	void InstanceItemData::serialise(QJsonObject& json) const
	{
		QJsonArray json_items;
		for (auto item : m_items)
		{
			json_items << item->m_entity->m_type_name;
		}

		json["Items"] = json_items;
		json["Location"] = (m_location != nullptr ? m_location->m_entity->m_type_name : QString());
		json["LocationEntrance"] = (m_location_entrance != nullptr ? m_location_entrance->m_type_name : QString());
		json["Cleared"] = m_cleared;
	}

	Result InstanceItemData::deserialise(const QJsonObject& json, int /*version*/, const EntityDatabase& entity_db, const ItemDatabase& item_db, const LocationDatabase& location_db)
	{
		Result result;

		QJsonValue jval_items;
		result << json_read(json, "Items", jval_items);

		for (auto jval_item : jval_items.toArray())
		{
			auto item_name = jval_item.toString();
			auto item = item_db.get_item(item_name);

			if (item != nullptr)
			{
				m_items << item;
			}
			else
			{
				result << ResultEntry(ResultType::Warning, "Item not found: " + item_name);
			}
		}

		auto location_name = json["Location"].toString();
		m_location = location_db.get_location(location_name);

		if (!location_name.isEmpty() && m_location == nullptr)
		{
			result << ResultEntry(ResultType::Warning, "Location not found: " + location_name);
		}

		auto entrance_name = json["LocationEntrance"].toString();
		m_location_entrance = entity_db.get_entity(entrance_name);

		if (!entrance_name.isEmpty() && m_location_entrance == nullptr)
		{
			result << ResultEntry(ResultType::Warning, "Location entrance not found: " + entrance_name);
		}

		result << json_read(json, "Cleared", m_cleared, false);

		return result;
	}



	//================================================================================
	// Connection
	//================================================================================

	void InstanceConnectionData::serialise(QJsonObject& json) const
	{
		QJsonArray json_items;
		for (auto item : m_items)
		{
			json_items << item->get().m_schema_item->get().m_name;
		}

		json["Items"] = json_items;
	}

	Result InstanceConnectionData::deserialise(const QJsonObject& json, int /*version*/, Instance& instance)
	{
		Result result;

		QJsonValue jval_items;
		result << json_read(json, "Items", jval_items);
		auto json_items = jval_items.toArray();

		for (auto json_item : json_items)
		{
			auto item_name = json_item.toString();
			auto instance_items = instance.items();

			auto it = std::find_if(instance_items.begin(), instance_items.end(), [item_name] (InstanceItemPtr item)
			{
				return (item->get().m_schema_item->get().m_name == item_name);
			});

			if (it != instance_items.end())
			{
				m_items << *it;
			}
			else
			{
				result << ResultEntry(ResultType::Warning, "Schema item not found: " + item_name);
			}
		}		

		return result;
	}



	//================================================================================
	// Progress Item
	//================================================================================

	InstanceProgressItemData::InstanceProgressItemData()
		: m_num(1)
	{
	}

	void InstanceProgressItemData::serialise(QJsonObject& json) const
	{
		json["Item"] = m_item->m_entity->m_type_name;
		json["Num"] = m_num;
	}

	Result InstanceProgressItemData::deserialise(const QJsonObject& json, int /*version*/, const ItemDatabase& item_db)
	{
		Result result;

		auto item_name = json["Item"].toString();
		m_item = item_db.get_item(item_name);

		if (!item_name.isEmpty() && m_item == nullptr)
		{
			result << ResultEntry(ResultType::Warning, "Item not found: " + item_name);
		}

		result << json_read(json, "Num", m_num, 0);

		return result;
	}



	//================================================================================
	// Progress Location
	//================================================================================

	InstanceProgressLocationData::InstanceProgressLocationData()
		: m_num_items(0)
		, m_num_keys_current(0)
		, m_num_keys_total(0)
		, m_has_map(false)
		, m_has_compass(false)
		, m_has_big_key(false)
		, m_is_pendant(false)
		, m_is_pendant_green(false)
		, m_is_crystal(false)
		, m_is_crystal_red(false)
		, m_cleared(false)
	{
	}

	void InstanceProgressLocationData::serialise(QJsonObject& json) const
	{
		json["Location"] = m_location->m_entity->m_type_name;
		json["NumItems"] = m_num_items;
		json["NumKeysCurrent"] = m_num_keys_current;
		json["NumKeysTotal"] = m_num_keys_total;
		json["HasMap"] = m_has_map;
		json["HasCompass"] = m_has_compass;
		json["HasBigKey"] = m_has_big_key;
		json["IsPendant"] = m_is_pendant;
		json["IsPendantGreen"] = m_is_pendant_green;
		json["IsCrystal"] = m_is_crystal;
		json["IsCrystalRed"] = m_is_crystal_red;
		json["Cleared"] = m_cleared;
	}

	Result InstanceProgressLocationData::deserialise(const QJsonObject& json, int /*version*/, const LocationDatabase& location_db)
	{
		Result result;

		QString location_name;
		result << json_read(json, "Location", location_name, QString());
		m_location = location_db.get_location(location_name);

		if (!location_name.isEmpty() && m_location == nullptr)
		{
			result << ResultEntry(ResultType::Warning, "Location not found: " + location_name);
		}

		result << json_read(json, "NumItems", m_num_items, 0);
		result << json_read(json, "NumKeysCurrent", m_num_keys_current, 0);
		result << json_read(json, "NumKeysTotal", m_num_keys_total, 0);
		result << json_read(json, "HasMap", m_has_map, false);
		result << json_read(json, "HasCompass", m_has_compass, false);
		result << json_read(json, "HasBigKey", m_has_big_key, false);
		result << json_read(json, "IsPendant", m_is_pendant, false);
		result << json_read(json, "IsPendantGreen", m_is_pendant_green, false);
		result << json_read(json, "IsCrystal", m_is_crystal, false);
		result << json_read(json, "IsCrystalRed", m_is_crystal_red, false);
		result << json_read(json, "Cleared", m_cleared, false);

		return result;
	}
}