// Project includes
#include "Data/Schema/SchemaData.h"
#include "Data/Schema/Schema.h"
#include "Utility/JSON.h"
#include "Utility/Result.h"

// Qt includes
#include <QJsonArray>
#include <QJsonObject>


namespace Utility
{
	//================================================================================
	// Enum Reflection
	//================================================================================

	DEFINE_ENUM_REFLECTION(LTTPMapTracker::SchemaRuleType)
	{
		//	Type														Type Name			Display Name
		{	LTTPMapTracker::SchemaRuleType::ProgressItem,				"ProgressItem",		"Progress Item"		},
		{	LTTPMapTracker::SchemaRuleType::ProgressLocation,			"ProgressLocation",	"Progress Location"	},
		{	LTTPMapTracker::SchemaRuleType::ProgressSpecial,			"ProgressSpecial",	"Progress Special"	},
		{	LTTPMapTracker::SchemaRuleType::SchemaRule,					"SchemaRule",		"Schema Rule"		},
		{	LTTPMapTracker::SchemaRuleType::SchemaItem,					"SchemaItem",		"Schema Item"		},
		{	LTTPMapTracker::SchemaRuleType::SchemaRegion,				"SchemaRegion",		"Schema Region"		},
		{	LTTPMapTracker::SchemaRuleType::Inaccessible,				"Inaccessible",		"Inaccessible"		}
	};

	DEFINE_ENUM_REFLECTION(LTTPMapTracker::SchemaRuleTypeProgressSpecial)
	{
		//	Type														Type Name			Display Name
		{	LTTPMapTracker::SchemaRuleTypeProgressSpecial::Pendant1,	"Pendant1",		"Pendant 1 (Green)"		},
		{	LTTPMapTracker::SchemaRuleTypeProgressSpecial::Pendant2,	"Pendant2",		"Pendant 2"				},
		{	LTTPMapTracker::SchemaRuleTypeProgressSpecial::Pendant3,	"Pendant3",		"Pendant 3"				},
		{	LTTPMapTracker::SchemaRuleTypeProgressSpecial::Crystal5,	"Crystal5",		"Crystal 5 (Red)"		},
		{	LTTPMapTracker::SchemaRuleTypeProgressSpecial::Crystal6,	"Crystal6",		"Crystal 6 (Red)"		},
		{	LTTPMapTracker::SchemaRuleTypeProgressSpecial::Crystal7,	"Crystal7",		"Crystals (All)"		}
	};

	DEFINE_ENUM_REFLECTION(LTTPMapTracker::SchemaRuleAccessType)
	{
		//	Type														Type Name			Display Name
		{	LTTPMapTracker::SchemaRuleAccessType::Entrance,				"Entrance",			"Entrance"			},
		{	LTTPMapTracker::SchemaRuleAccessType::EntranceAndExit,		"EntranceAndExit",	"Entrance & Exit"	}
	};

	DEFINE_ENUM_REFLECTION(LTTPMapTracker::SchemaRuleOperator)
	{
		//	Type														Type Name			Display Name
		{	LTTPMapTracker::SchemaRuleOperator::Or,						"Or",				"Or"				},
		{	LTTPMapTracker::SchemaRuleOperator::And,					"And",				"And"				}
	};

	DEFINE_ENUM_REFLECTION(LTTPMapTracker::SchemaItemMap)
	{
		//	Type														Type Name			Display Name
		{	LTTPMapTracker::SchemaItemMap::LightWorld,					"LW",				"Light World"		},
		{	LTTPMapTracker::SchemaItemMap::DarkWorld,					"DW",				"Dark World"		}
	};
}


namespace LTTPMapTracker
{
	//================================================================================
	// Rule
	//================================================================================

	void SchemaRuleEntry::serialise(QJsonObject& json) const
	{
		json["Type"] = EnumReflection<SchemaRuleType>::info(m_type).m_type_name;
		json["Value"] = m_value;
		json["BracketsOpen"] = m_brackets_open;
		json["BracketsClose"] = m_brackets_close;
		json["Operator"] = EnumReflection<SchemaRuleOperator>::info(m_operator).m_type_name;
	}

	Result SchemaRuleEntry::deserialise(const QJsonObject& json, int /*version*/)
	{
		Result result;

		QString type_name;
		result << json_read(json, "Type", type_name, QString());
		auto type_info = EnumReflection<SchemaRuleType>::info(type_name);
		m_type = (type_info != nullptr ? type_info->m_type : SchemaRuleType::ProgressItem);

		QString operator_name;
		result << json_read(json, "Operator", operator_name, QString());
		auto operator_info = EnumReflection<SchemaRuleOperator>::info(operator_name);
		m_operator = (operator_info != nullptr ? operator_info->m_type : SchemaRuleOperator::Or);

		result << json_read(json, "Value", m_value, QString());
		result << json_read(json, "BracketsOpen", m_brackets_open, 0);
		result << json_read(json, "BracketsClose", m_brackets_close, 0);

		return result;
	}

	//--------------------------------------------------------------------------------

	void SchemaRuleData::serialise(QJsonObject& json) const
	{
		QJsonArray json_entries;
		for (auto& entry : m_entries)
		{
			QJsonObject json_entry;
			entry.serialise(json_entry);
			json_entries << json_entry;
		}

		json["Name"] = m_name;
		json["RuleEntries"] = json_entries;
	}

	Result SchemaRuleData::deserialise(const QJsonObject& json, int version, Schema& /*schema*/)
	{
		Result result;

		result << json_read(json, "Name", m_name, m_name);

		QJsonValue jval_entries;
		result << json_read(json, "RuleEntries", jval_entries);
		auto json_entries = jval_entries.toArray();

		for (auto jval_entry : json_entries)
		{
			SchemaRuleEntry entry;
			result << entry.deserialise(jval_entry.toObject(), version);
			m_entries << entry;
		}

		return result;
	}



	//================================================================================
	// Region
	//================================================================================

	SchemaRegionData::SchemaRegionData()
		: m_color(255, 255, 255)
		, m_accessible(false)
		, m_accessible_cached(false)
	{
	}

	void SchemaRegionData::serialise(QJsonObject& json) const
	{
		json["Name"] = m_name;
		json["Color"] = m_color.name();
		json["Rule"] = (m_rule != nullptr ? m_rule->get().m_name : QString());
	}

	Result SchemaRegionData::deserialise(const QJsonObject& json, int /*version*/, Schema& schema)
	{
		Result result;

		result << json_read(json, "Name", m_name, m_name);

		QString color_name;
		result << json_read(json, "Color", color_name, m_color.name());
		m_color.setNamedColor(color_name);

		QString rule_name;
		result << json_read(json, "Rule", rule_name, QString());
		m_rule = schema.rules().find(rule_name);

		if (!rule_name.isEmpty() && m_rule == nullptr)
		{
			result << ResultEntry(ResultType::Warning, "Rule not found: " + rule_name);
		}

		return result;
	}



	//================================================================================
	// Item
	//================================================================================

	SchemaItemData::SchemaItemData()
		: m_map(SchemaItemMap::LightWorld)
		, m_rule_access(SchemaRuleAccessType::Entrance)
	{
	}

	void SchemaItemData::serialise(QJsonObject& json) const
	{
		json["Name"] = m_name;
		json["Map"] = EnumReflection<SchemaItemMap>::info(m_map).m_type_name;
		json["PositionX"] = m_position.x();
		json["PositionY"] = m_position.y();
		json["Region"] = (m_region != nullptr ? m_region->get().m_name : QString());
		json["Rule"] = (m_rule != nullptr ? m_rule->get().m_name : QString());
		json["RuleAccess"] = EnumReflection<SchemaRuleAccessType>::info(m_rule_access).m_type_name;
	}

	Result SchemaItemData::deserialise(const QJsonObject& json, int /*version*/, Schema& schema)
	{
		Result result;

		result << json_read(json, "Name", m_name, m_name);

		int x, y;
		result << json_read(json, "PositionX", x, 0);
		result << json_read(json, "PositionY", y, 0);
		m_position = QPoint(x, y);

		QString map_name;
		result << json_read(json, "Map", map_name, QString());
		auto map_info = EnumReflection<SchemaItemMap>::info(map_name);
		m_map = (map_info != nullptr ? map_info->m_type : SchemaItemMap::LightWorld);

		QString region_name;
		result << json_read(json, "Region", region_name, QString());
		m_region = schema.regions().find(region_name);

		if (!region_name.isEmpty() && m_region == nullptr)
		{
			result << ResultEntry(ResultType::Warning, "Region not found: " + region_name);
		}

		QString rule_name;
		result << json_read(json, "Rule", rule_name, QString());
		m_rule = schema.rules().find(rule_name);

		if (!rule_name.isEmpty() && m_rule == nullptr)
		{
			result << ResultEntry(ResultType::Warning, "Rule not found: " + rule_name);
		}

		QString rule_access_name;
		result << json_read(json, "RuleAccess", rule_access_name, QString());
		auto rule_access_info = EnumReflection<SchemaRuleAccessType>::info(rule_access_name);
		m_rule_access = (rule_access_info != nullptr ? rule_access_info->m_type : SchemaRuleAccessType::Entrance);

		return result;
	}
}