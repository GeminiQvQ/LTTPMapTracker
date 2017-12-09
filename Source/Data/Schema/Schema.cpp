// Project includes
#include "Data/Schema/Schema.h"
#include "Utility/JSON.h"

// Qt includes
#include <QFile>
#include <QJsonArray>
#include <QVector>


namespace LTTPMapTracker
{
	//================================================================================
	// Utility
	//================================================================================

	template <typename T>
	QString create_unique_name(QString base, QVector<T> container)
	{
		for (int i = 1; i < INT_MAX; ++i)
		{
			auto name = QString("%1 %2").arg(base).arg(i);
			
			auto it = std::find_if(container.begin(), container.end(), [name] (const T& entry)
			{
				return (entry->get().m_name == name);
			});

			if (it == container.end())
			{
				return name;
			}
		}

		Q_ASSERT(false);

		return base;
	}

	template <typename T>
	bool compare_name(std::shared_ptr<const T> data, QString name)
	{
		return (data->get().m_name == name);
	}
}


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct Schema::Internal
	{
		QString			m_filename;
		bool			m_dirty;

		SchemaItems		m_items;
		SchemaRegions	m_regions;
		SchemaRules		m_rules;

		Internal(Schema& schema)
			: m_dirty(false)
			, m_items(std::bind(&Schema::create_item, &schema), std::bind(&Schema::create_item, &schema), compare_name<SchemaItem>)
			, m_regions(std::bind(&Schema::create_region, &schema), std::bind(&Schema::create_region, &schema), compare_name<SchemaRegion>)
			, m_rules(std::bind(&Schema::create_rule, &schema), std::bind(&Schema::create_rule, &schema), compare_name<SchemaRule>)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	Schema::Schema()
		: QObject(nullptr)
		, m_internal(std::make_unique<Internal>(*this))
	{
		// Signals.
		connect(&m_internal->m_items, &SchemaItems::signal_to_be_added, this, &Schema::set_dirty);
		connect(&m_internal->m_items, &SchemaItems::signal_added, this, &Schema::set_dirty);
		connect(&m_internal->m_items, &SchemaItems::signal_to_be_removed, this, &Schema::set_dirty);
		connect(&m_internal->m_items, &SchemaItems::signal_removed, this, &Schema::set_dirty);
		connect(&m_internal->m_items, &SchemaItems::signal_modified, this, &Schema::set_dirty);
		connect(&m_internal->m_regions, &SchemaRegions::signal_to_be_added, this, &Schema::set_dirty);
		connect(&m_internal->m_regions, &SchemaRegions::signal_added, this, &Schema::set_dirty);
		connect(&m_internal->m_regions, &SchemaRegions::signal_to_be_removed, this, &Schema::set_dirty);
		connect(&m_internal->m_regions, &SchemaRegions::signal_removed, this, &Schema::set_dirty);
		connect(&m_internal->m_regions, &SchemaRegions::signal_modified, this, &Schema::slot_region_modified);
		connect(&m_internal->m_rules, &SchemaRules::signal_to_be_added, this, &Schema::set_dirty);
		connect(&m_internal->m_rules, &SchemaRules::signal_added, this, &Schema::set_dirty);
		connect(&m_internal->m_rules, &SchemaRules::signal_to_be_removed, this, &Schema::set_dirty);
		connect(&m_internal->m_rules, &SchemaRules::signal_removed, this, &Schema::set_dirty);
		connect(&m_internal->m_rules, &SchemaRules::signal_modified, this, &Schema::slot_rule_modified);
	}

	Schema::~Schema()
	{
	}



	//================================================================================
	// Load & Save
	//================================================================================

	Result Schema::save()
	{
		return (!m_internal->m_filename.isEmpty() ? save(m_internal->m_filename) : Result(ResultType::Error, "No filename set."));
	}

	Result Schema::save(QString filename)
	{
		// Serialize.
		QJsonObject json;

		json["Version"] = 1;

		m_internal->m_items.serialise("Items", json);
		m_internal->m_regions.serialise("Regions", json);
		m_internal->m_rules.serialise("Rules", json);

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

	Result Schema::load(QString filename)
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
		
		result << m_internal->m_rules.deserialise("Rules", json, version, *this);
		result << m_internal->m_regions.deserialise("Regions", json, version, *this);
		result << m_internal->m_items.deserialise("Items", json, version, *this);
		
		m_internal->m_filename = filename;
		m_internal->m_dirty = false;

		emit signal_dirty_state_changed(false);

		return result;
	}



	//================================================================================
	// Properties
	//================================================================================

	QString Schema::get_filename()
	{
		return m_internal->m_filename;
	}

	bool Schema::is_dirty() const
	{
		return m_internal->m_dirty;
	}



	//================================================================================
	// Data
	//================================================================================

	SchemaItems& Schema::items()
	{
		return m_internal->m_items;
	}

	const SchemaItems& Schema::items() const
	{
		return m_internal->m_items;
	}

	SchemaRegions& Schema::regions()
	{
		return m_internal->m_regions;
	}

	const SchemaRegions& Schema::regions() const
	{
		return m_internal->m_regions;
	}

	SchemaRules& Schema::rules()
	{
		return m_internal->m_rules;
	}

	const SchemaRules& Schema::rules() const
	{
		return m_internal->m_rules;
	}

	

	//================================================================================
	// Data Slots
	//================================================================================

	void Schema::slot_region_modified(int index)
	{
		auto region = m_internal->m_regions.get()[index];

		for (auto item : m_internal->m_items.get())
		{
			if (item->get().m_region == region)
			{
				item->signal_modified();
			}
		}

		set_dirty();
	}

	void Schema::slot_rule_modified(int index)
	{
		auto rule = m_internal->m_rules.get()[index];

		for (auto region : m_internal->m_regions.get())
		{
			if (region->get().m_rule == rule)
			{
				region->signal_modified();
			}
		}

		for (auto item : m_internal->m_items.get())
		{
			if (item->get().m_rule == rule)
			{
				item->signal_modified();
			}
		}

		set_dirty();
	}



	//================================================================================
	// Helpers
	//================================================================================

	SchemaItemPtr Schema::create_item()
	{
		auto item = std::make_shared<SchemaItem>();
		auto data = item->get();
		data.m_name = create_unique_name("Item", m_internal->m_items.get());
		item->set(data);
		return item;
	}

	SchemaRegionPtr Schema::create_region()
	{
		auto region = std::make_shared<SchemaRegion>();
		auto data = region->get();
		data.m_name = create_unique_name("Region", m_internal->m_regions.get());
		region->set(data);
		return region;
	}

	SchemaRulePtr Schema::create_rule()
	{
		auto rule = std::make_shared<SchemaRule>();
		auto data = rule->get();
		data.m_name = create_unique_name("Rule", m_internal->m_rules.get());
		rule->set(data);
		return rule;
	}

	void Schema::set_dirty()
	{
		m_internal->m_dirty = true;
		emit signal_dirty_state_changed(true);
	}
}