// Project includes
#include "Data/Instance/Instance.h"
#include "Data/Instance/InstanceRuleParser.h"
#include "Data/Schema/Schema.h"
#include "Data/DataModel.h"
#include "Utility/File.h"
#include "Utility/JSON.h"

// Qt includes
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QVector>


namespace LTTPMapTracker
{
	//================================================================================
	// Utility
	//================================================================================

	bool compare_connection_item(InstanceConnectionCPtr connection, InstanceItemPtr item)
	{
		return connection->get().m_items.contains(item);
	}

	bool compare_progress_item(InstanceProgressItemCPtr progress_item, EntityCPtr entity)
	{
		return (progress_item->get().m_item->m_entity == entity);
	}

	bool compare_progress_location(InstanceProgressLocationCPtr progress_location, EntityCPtr entity)
	{
		return (progress_location->get().m_location->m_entity == entity);
	}
}


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct Instance::Internal
	{
		const DataModel&			m_data_model;
		SchemaCPtr					m_schema;
		QVector<InstanceItemPtr>	m_items;
		QVector<InstanceItemCPtr>	m_citems;
		InstanceConnections			m_connections;
		InstanceProgressItems		m_progress_items;
		InstanceProgressLocations	m_progress_locations;

		QString						m_filename;
		QString						m_filename_auto;
		bool						m_dirty;

		Internal(Instance& instance, const DataModel& data_model, SchemaCPtr schema)
			: m_data_model(data_model)
			, m_schema(schema)
			, m_connections(std::bind(&Instance::create_connection, &instance, std::placeholders::_1), std::bind(&Instance::create_connection_empty, &instance), compare_connection_item)
			, m_progress_items(std::bind(&Instance::create_progress_item, &instance, std::placeholders::_1), std::bind(&Instance::create_progress_item_empty, &instance), compare_progress_item)
			, m_progress_locations(std::bind(&Instance::create_progress_location, &instance, std::placeholders::_1), std::bind(&Instance::create_progress_location_empty, &instance), compare_progress_location)
			, m_filename_auto(get_absolute_path(QString("Data/Instances/AutoSave/%1.instance.json").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"))))
			, m_dirty(false)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	Instance::Instance(const DataModel& data_model, SchemaCPtr schema)
		: m_internal(std::make_unique<Internal>(*this, data_model, schema))
	{
		// Enumerate data.
		for (auto schema_item : schema->items().get())
		{
			auto item = create_item(schema_item);
			m_internal->m_items << item;
			m_internal->m_citems << item;
		}

		for (auto location : m_internal->m_data_model.get_location_db().get_locations())
		{
			m_internal->m_progress_locations.add(location);
		}

		cache_accessibility();

		// Signals.
		connect(&m_internal->m_connections, &InstanceConnections::signal_added, this, &Instance::set_dirty);
		connect(&m_internal->m_connections, &InstanceConnections::signal_removed, this, &Instance::set_dirty);
		connect(&m_internal->m_connections, &InstanceConnections::signal_modified, this, &Instance::set_dirty);
		connect(&m_internal->m_progress_items, &InstanceProgressItems::signal_added, this, &Instance::set_dirty);
		connect(&m_internal->m_progress_items, &InstanceProgressItems::signal_removed, this, &Instance::set_dirty);
		connect(&m_internal->m_progress_items, &InstanceProgressItems::signal_modified, this, &Instance::set_dirty);
		connect(&m_internal->m_progress_locations, &InstanceProgressLocations::signal_added, this, &Instance::set_dirty);
		connect(&m_internal->m_progress_locations, &InstanceProgressLocations::signal_removed, this, &Instance::set_dirty);
		connect(&m_internal->m_progress_locations, &InstanceProgressLocations::signal_modified, this, &Instance::set_dirty);
	}

	Instance::~Instance()
	{
	}



	//================================================================================
	// Save & Load
	//================================================================================

	Result Instance::save()
	{
		return (!m_internal->m_filename.isEmpty() ? save(m_internal->m_filename) : Result(ResultType::Error, "No filename set."));
	}

	Result Instance::save(QString filename)
	{
		// Serialize.
		QJsonObject json;

		json["Version"] = 1;

		QJsonObject json_items;
		for (auto item : m_internal->m_items)
		{
			QJsonObject json_item;
			item->serialise(json_item);
			json_items[item->get().m_schema_item->get().m_name] = json_item;
		}
		json["Items"] = json_items;

		m_internal->m_connections.serialise("Connections", json);
		m_internal->m_progress_items.serialise("ProgressItems", json);
		m_internal->m_progress_locations.serialise("ProgressLocations", json);

		// Write data.
		auto save_result = json_save(json, filename);
		if (!save_result)
		{
			return save_result;
		}

		m_internal->m_filename = filename;
		m_internal->m_dirty = false;

		emit signal_dirty_state_changed(false);

		return Result();
	}

	Result Instance::save_auto()
	{
		auto filename = m_internal->m_filename;
		bool dirty = m_internal->m_dirty;

		QDir().mkpath(m_internal->m_filename_auto.section("/", 0, -2));

		blockSignals(true);
		auto result = save(m_internal->m_filename_auto);
		blockSignals(false);

		m_internal->m_filename = filename;
		m_internal->m_dirty = dirty;

		return result;
	}

	Result Instance::load(QString filename)
	{
		// Load data.
		QJsonObject json;
		
		auto load_result = json_load(json, filename);
		if (!load_result)
		{
			return load_result;
		}

		// Deserialize.
		int version = 0;
		if (!json_read(json, "Version", version, 0))
		{
			return Result(false, "Unable to read version.");
		}

		Result result;

		QJsonValue jval_items;
		result << json_read(json, "Items", jval_items);
		auto json_items = jval_items.toObject();

		for (auto item : m_internal->m_items)
		{
			auto it = json_items.find(item->get().m_schema_item->get().m_name);
			if (it != json_items.end())
			{
				result << item->deserialise(it->toObject(), version, m_internal->m_data_model.get_entity_db(), m_internal->m_data_model.get_item_db(), m_internal->m_data_model.get_location_db());
			}
			else
			{
				result << ResultEntry(ResultType::Warning, "Item not found: " + item->get().m_schema_item->get().m_name);
			}
		}

		result << m_internal->m_connections.deserialise("Connections", json, version, *this);
		result << m_internal->m_progress_items.deserialise("ProgressItems", json, version, m_internal->m_data_model.get_item_db());
		result << m_internal->m_progress_locations.deserialise("ProgressLocations", json, version, m_internal->m_data_model.get_location_db());
		
		cache_accessibility();

		m_internal->m_filename = filename;
		m_internal->m_dirty = false;

		emit signal_dirty_state_changed(false);

		return result;
	}

	Result Instance::load_template(QString filename)
	{
		auto load_result = load(filename);
		m_internal->m_filename = QString();
		emit signal_dirty_state_changed(false);
		return load_result;
	}



	//================================================================================
	// Properties
	//================================================================================

	QString Instance::get_filename()
	{
		return m_internal->m_filename;
	}

	bool Instance::is_dirty() const
	{
		return m_internal->m_dirty;
	}



	//================================================================================
	// Data
	//================================================================================

	const QVector<InstanceItemPtr>& Instance::items()
	{
		return m_internal->m_items;
	}

	const QVector<InstanceItemCPtr>& Instance::items() const
	{
		return m_internal->m_citems;
	}

	InstanceConnections& Instance::connections()
	{
		return m_internal->m_connections;
	}

	const InstanceConnections& Instance::connections() const
	{
		return m_internal->m_connections;
	}

	InstanceProgressItems& Instance::progress_items()
	{
		return m_internal->m_progress_items;
	}

	const InstanceProgressItems& Instance::progress_items() const
	{
		return m_internal->m_progress_items;
	}

	InstanceProgressLocations& Instance::progress_locations()
	{
		return m_internal->m_progress_locations;
	}

	const InstanceProgressLocations& Instance::progress_locations() const
	{
		return m_internal->m_progress_locations;
	}



	//================================================================================
	// Accessors
	//================================================================================

	SchemaCPtr Instance::get_schema() const
	{
		return m_internal->m_schema;
	}



	//================================================================================
	// Helpers
	//================================================================================

	InstanceItemPtr Instance::create_item(SchemaItemCPtr schema_item)
	{
		auto item = std::make_shared<InstanceItem>();
		auto data = item->get();
		data.m_schema_item = schema_item;
		item->set(data);
		QObject::connect(item.get(), &InstanceItem::signal_modified, this, &Instance::set_dirty);
		return item;
	}

	InstanceConnectionPtr Instance::create_connection_empty()
	{
		return std::make_shared<InstanceConnection>();
	}

	InstanceConnectionPtr Instance::create_connection(QVector<InstanceItemPtr> instance_items)
	{
		auto connection = std::make_shared<InstanceConnection>();
		auto data = connection->get();
		data.m_items = instance_items;
		connection->set(data);
		return connection;
	}

	InstanceProgressItemPtr Instance::create_progress_item_empty()
	{
		return std::make_shared<InstanceProgressItem>();
	}

	InstanceProgressItemPtr Instance::create_progress_item(ItemCPtr item)
	{
		auto progress_item = std::make_shared<InstanceProgressItem>();
		auto data = progress_item->get();
		data.m_item = item;
		progress_item->set(data);
		return progress_item;
	}

	InstanceProgressLocationPtr Instance::create_progress_location_empty()
	{
		return std::make_shared<InstanceProgressLocation>();
	}

	InstanceProgressLocationPtr Instance::create_progress_location(LocationCPtr location)
	{
		auto progress_location = std::make_shared<InstanceProgressLocation>();
		auto data = progress_location->get();
		data.m_location = location;
		progress_location->set(data);
		return progress_location;
	}

	//--------------------------------------------------------------------------------

	void Instance::set_dirty()
	{
		cache_accessibility();
		m_internal->m_dirty = true;
		emit signal_dirty_state_changed(true);
	}

	void Instance::cache_accessibility()
	{
		// Schema regions.
		for (auto region : m_internal->m_schema->regions().get())
		{
			region->get().m_accessible_cached = false;
		}

		for (auto region : m_internal->m_schema->regions().get())
		{
			if (region->get().m_name == "DW - Main - West")
			{
				int a = 0;
				++a;
			}

			region->get().m_accessible = match_rule(*this, region);
			region->get().m_accessible_cached = true;
		}

		// Instance items.
		for (auto item : m_internal->m_items)
		{
			item->get().m_accessible_cached = false;
		}

		for (auto item : m_internal->m_items)
		{
			if (item->get().m_schema_item->get().m_name == "Village of Outcasts - Chest Game")
			{
				int a = 0;
				++a;
			}


			item->get().m_accessible = match_rule(*this, item);
			item->get().m_accessible_cached = true;
		}

		emit signal_accessibility_cached();
	}
}