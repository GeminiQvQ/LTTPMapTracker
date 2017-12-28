// Project includes
#include "Data/Database/LocationDatabase.h"
#include "Data/Database/ItemDatabase.h"
#include "Data/Instance/Instance.h"
#include "Data/Instance/InstanceRuleParser.h"
#include "Data/Schema/SchemaData.h"
#include "Data/DataModel.h"
#include "Utility/JSON.h"


// Qt includes
#include <QJsonArray>
#include <QMap>


namespace LTTPMapTracker
{
	//================================================================================
	// Constants
	//================================================================================

	static const QString s_db_filename = "Data/LocationDatabase.json";



	//================================================================================
	// Location
	//================================================================================

	Location::Location()
		: m_treasure(0)
		, m_treasure_keys(0)
		, m_treasure_etc(0)
		, m_is_startpos(false)
	{
	}



	//================================================================================
	// Loading
	//================================================================================

	Result LocationDatabase::load(const EntityDatabase& entity_db)
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
		QMap<QString, std::shared_ptr<Location>> locations;

		QJsonValue jval_locations;
		result << json_read(json, "Locations", jval_locations);
		auto json_locations = jval_locations.toArray();

		for (auto jval_location : json_locations)
		{
			auto json_location = jval_location.toObject();

			QString entity_name;
			if (!(result << json_read(json_location, "Entity", entity_name, QString())))
			{
				continue;
			}

			if (locations.contains(entity_name))
			{
				result << ResultEntry(ResultType::Warning, "Duplicate location type name: " + entity_name);
				continue;
			}

			auto entity = entity_db.get_entity(entity_name);
			if (entity == nullptr)
			{
				result << ResultEntry(ResultType::Warning, "Entity not found: " + entity_name);
				continue;
			}

			auto location = std::make_shared<Location>();
			location->m_entity = entity;
			locations.insert(entity_name, location);

			json_read(json_location, "Treasure", location->m_treasure, 0);
			json_read(json_location, "TreasureKeys", location->m_treasure_keys, 0);
			json_read(json_location, "TreasureEtc", location->m_treasure_etc, 0);
			json_read(json_location, "IsStartPos", location->m_is_startpos, false);

			QJsonValue jval_required_entities;
			json_read(json_location, "Requirements", jval_required_entities);
			result << deserialise_location_requirements(location->m_requirements, jval_required_entities.toArray(), entity_db);

			QJsonValue jval_entrances;
			json_read(json_location, "Entrances", jval_entrances);
			for (auto jval_entrance : jval_entrances.toArray())
			{
				auto entrance_entity_name = jval_entrance.toString();
				auto entrance_entity = entity_db.get_entity(entrance_entity_name);

				if (entrance_entity == nullptr)
				{
					result << ResultEntry(ResultType::Warning, "Entity not found: " + entrance_entity_name);
					continue;
				}

				location->m_entrances << entrance_entity;
			}

			QJsonValue jval_connections;
			json_read(json_location, "Connections", jval_connections);
			for (auto jval_connection : jval_connections.toArray())
			{
				auto json_connection = jval_connection.toArray();
				if (json_connection.size() != 3)
				{
					result << ResultEntry(ResultType::Warning, "Malformed connection data for location: " + entity_name);
					continue;
				}

				auto entrance1 = json_connection[0].toString();
				auto entrance1_entity = entity_db.get_entity(entrance1);
				if (entrance1_entity == nullptr)
				{
					result << ResultEntry(ResultType::Warning, "Entity not found: " + entrance1);
					continue;
				}

				auto entrance2 = json_connection[1].toString();
				auto entrance2_entity = entity_db.get_entity(entrance2);
				if (entrance2_entity == nullptr)
				{
					result << ResultEntry(ResultType::Warning, "Entity not found: " + entrance2);
					continue;
				}

				if (!location->m_entrances.contains(entrance1_entity) || !location->m_entrances.contains(entrance2_entity))
				{
					result << ResultEntry(ResultType::Warning, "Malformed connection data for location: " + entity_name);
					continue;
				}

				LocationConnection connection;
				connection.m_entrances << entrance1_entity << entrance2_entity;
				result << deserialise_location_requirements(connection.m_requirements, json_connection[2].toArray(), entity_db);
				location->m_connections << connection;
			}
		}

		// Store locations.
		std::copy(locations.begin(), locations.end(), std::back_inserter(m_locations));

		return result;
	}



	//================================================================================
	// Entity
	//================================================================================

	LocationCPtr LocationDatabase::get_location(QString type_name) const
	{
		auto it = std::find_if(m_locations.begin(), m_locations.end(), [type_name] (LocationCPtr location)
		{
			return (location->m_entity->m_type_name == type_name);
		});
		return (it != m_locations.end() ? *it : nullptr);
	}

	LocationCPtr LocationDatabase::get_location(EntityCPtr entity) const
	{
		auto it = std::find_if(m_locations.begin(), m_locations.end(), [entity] (LocationCPtr location)
		{
			return (location->m_entity == entity);
		});	
		return (it != m_locations.end() ? *it : nullptr);
	}

	LocationList LocationDatabase::get_locations() const
	{
		return m_locations;
	}
}


namespace LTTPMapTracker
{
	//================================================================================
	// Utility
	//================================================================================

	// [TODO]
	// I feel this location requirement stuff doesn't really belong here and is kind of a hack job.
	// It would be good to standardize all rule logic parsing.

	Result deserialise_location_requirements(QVector<LocationRequirement>& requirements, const QJsonArray& json, const EntityDatabase& entity_db)
	{
		Result result;

		for (auto jval_required_entity : json)
		{
			QVector<LocationRequirementEntry> entries;

			QStringList requirement_entry_names;
			if (jval_required_entity.isArray())
			{
				for (auto jval_required_entity_part : jval_required_entity.toArray())
				{
					requirement_entry_names << jval_required_entity_part.toString();
				}
			}
			else
			{
				requirement_entry_names << jval_required_entity.toString();
			}

			for (auto requirement_entry_name : requirement_entry_names)
			{
				auto parts = requirement_entry_name.split(":");
				auto part_type = (parts.size() > 1 ? parts[0] : QString());
				auto part_name = (parts.size() > 1 ? parts[1] : parts[0]);

				LocationRequirementType type = LocationRequirementType::ProgressItem;
				if (part_type == "ProgressLocation")
				{
					type = LocationRequirementType::ProgressLocation;
				}
				if (part_type == "ProgressSpecial")
				{
					type = LocationRequirementType::ProgressSpecial;
				}

				bool optional = part_name.contains('?');
				part_name.remove('?');

				QVariant value;
				switch (type)
				{
				case LocationRequirementType::ProgressItem:
				case LocationRequirementType::ProgressLocation:
				default:
					{
						if (entity_db.get_entity(part_name) != nullptr)
						{
							value = part_name;
						}
					}
					break;

				case LocationRequirementType::ProgressSpecial:
					{
						auto info = EnumReflection<SchemaRuleTypeProgressSpecial>::info(part_name);
						if (info != nullptr)
						{
							value = (int)info->m_type;
						}
					}
					break;
				}

				if (value.isNull())
				{
					result << ResultEntry(ResultType::Warning, "Location requirement token not found: " + part_name);
					continue;
				}

				LocationRequirementEntry entry;
				entry.m_type = type;
				entry.m_value = value;
				entry.m_optional = optional;
				entries << entry;
			}

			if (!entries.isEmpty())
			{
				LocationRequirement requirement;
				requirement.m_entries = entries;
				requirements << requirement;
			}
		}

		return result;
	}

	LocationMatch match_location_requirement(const LocationRequirement& requirement, const Instance& instance)
	{
		auto result = LocationMatch::No;

		for (auto& entry : requirement.m_entries)
		{
			bool entry_result = false;

			switch (entry.m_type)
			{
			case LocationRequirementType::ProgressItem:
				{
					auto progress_items = instance.progress_items().get();
					entry_result = std::any_of(progress_items.begin(), progress_items.end(), [&entry] (InstanceProgressItemCPtr progress_item)
					{
						return (progress_item->get().m_item->m_entity->m_type_name == entry.m_value.toString());
					});
				}
				break;

			case LocationRequirementType::ProgressLocation:
				{
					auto progress_locations = instance.progress_locations().get();
					entry_result = std::any_of(progress_locations.begin(), progress_locations.end(), [&entry] (InstanceProgressLocationCPtr progress_location)
					{
						return (progress_location->get().m_location->m_entity->m_type_name == entry.m_value.toString() &&
								progress_location->get().m_cleared);
					});
				}
				break;

			case LocationRequirementType::ProgressSpecial:
				entry_result = match_rule(instance, (SchemaRuleTypeProgressSpecial)entry.m_value.toInt());
				break;
			}

			auto entry_match = (entry_result ? LocationMatch::Yes : (entry.m_optional ? LocationMatch::Maybe : LocationMatch::No));
			if (entry_match > result)
			{
				result = entry_match;
			}
		}

		return result;
	}

	LocationMatch match_location_requirements(const QVector<LocationRequirement>& requirements, const Instance& instance)
	{
		if (!requirements.isEmpty())
		{
			auto result = LocationMatch::Yes;

			for (auto& requirement : requirements)
			{
				auto requirement_result = match_location_requirement(requirement, instance);
				if (requirement_result < result)
				{
					result = requirement_result;
				}
			}

			return result;
		}
		else
		{
			return LocationMatch::Maybe;
		}
	}
}