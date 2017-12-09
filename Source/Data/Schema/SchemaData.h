#ifndef SCHEMA_DATA_H
#define SCHEMA_DATA_H

// Project includes
#include "Data/Schema/SchemaTypeInfo.h"
#include "Utility/DataWrapper.h"
#include "Utility/EnumReflection.h"

// Qt includes
#include <QColor>
#include <QString>
#include <QVariant>
#include <QVector>


namespace LTTPMapTracker
{
	// Rule
	//--------------------------------------------------------------------------------

	enum class SchemaRuleType
	{
		ProgressItem,
		ProgressLocation,
		ProgressSpecial,
		SchemaItem,
		SchemaRegion,
		SchemaRule,
		Inaccessible
	};

	enum class SchemaRuleTypeProgressSpecial
	{
		Pendant1,
		Pendant2,
		Pendant3,
		Crystal5,
		Crystal6,
		Crystal7
	};

	enum class SchemaRuleAccessType
	{
		Entrance,
		EntranceAndExit
	};

	enum class SchemaRuleOperator
	{
		Or,
		And
	};

	struct SchemaRuleEntry
	{
		SchemaRuleType		m_type;
		QString				m_value;
		SchemaRuleOperator	m_operator;
		int					m_brackets_open;
		int					m_brackets_close;

		void	serialise	(QJsonObject& json) const;
		Result	deserialise	(const QJsonObject& json, int version);
	};

	struct SchemaRuleData
	{
	public:
		QString						m_name;
		QVector<SchemaRuleEntry>	m_entries;

		void	serialise		(QJsonObject& json) const;
		Result	deserialise		(const QJsonObject& json, int version, Schema& schema);
	};

	class SchemaRule : public SerializableDataWrapper<SchemaRuleData> {};


	// Region
	//--------------------------------------------------------------------------------

	struct SchemaRegionData
	{
		QString		  m_name;
		QColor		  m_color;
		SchemaRulePtr m_rule;

		mutable bool  m_accessible;
		mutable bool  m_accessible_cached;

				SchemaRegionData	();
		void	serialise			(QJsonObject& json) const;
		Result	deserialise			(const QJsonObject& json, int version, Schema& schema);
	};

	class SchemaRegion : public SerializableDataWrapper<SchemaRegionData> {};


	// Item
	//--------------------------------------------------------------------------------

	enum class SchemaItemMap
	{
		LightWorld,
		DarkWorld
	};

	struct SchemaItemData
	{
		QString				 m_name;
		SchemaItemMap		 m_map;
		QPointF				 m_position;
		SchemaRegionPtr		 m_region;
		SchemaRulePtr		 m_rule;
		SchemaRuleAccessType m_rule_access;

				SchemaItemData	();
		void	serialise		(QJsonObject& json) const;
		Result	deserialise		(const QJsonObject& json, int version, Schema& schema);
	};

	class SchemaItem : public SerializableDataWrapper<SchemaItemData> {};
}

DECLARE_ENUM_REFLECTION(LTTPMapTracker::SchemaRuleType);
DECLARE_ENUM_REFLECTION(LTTPMapTracker::SchemaRuleTypeProgressSpecial);
DECLARE_ENUM_REFLECTION(LTTPMapTracker::SchemaRuleAccessType);
DECLARE_ENUM_REFLECTION(LTTPMapTracker::SchemaRuleOperator);
DECLARE_ENUM_REFLECTION(LTTPMapTracker::SchemaItemMap);

#endif