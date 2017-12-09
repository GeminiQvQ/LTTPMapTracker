#ifndef INSTANCE_DATA_H
#define INSTANCE_DATA_H

// Project includes
#include "Data/Instance/InstanceTypeInfo.h"
#include "Data/Schema/SchemaTypeInfo.h"
#include "Data/Database/EntityDatabase.h"
#include "Data/Database/ItemDatabase.h"
#include "Data/Database/LocationDatabase.h"
#include "Utility/DataWrapper.h"


namespace LTTPMapTracker
{
	// Item
	//--------------------------------------------------------------------------------

	struct InstanceItemData
	{
		SchemaItemCPtr		m_schema_item;
		QVector<ItemCPtr>	m_items;
		LocationCPtr		m_location;
		EntityCPtr			m_location_entrance;
		bool				m_cleared;

		mutable bool		m_accessible;
		mutable bool		m_accessible_cached;

				InstanceItemData	();
		void	serialise			(QJsonObject& json) const;
		Result	deserialise			(const QJsonObject& json, int version, const EntityDatabase& entity_db, const ItemDatabase& item_db, const LocationDatabase& location_db);
	};

	class InstanceItem : public SerializableDataWrapper<InstanceItemData> {};


	// Connection
	//--------------------------------------------------------------------------------

	struct InstanceConnectionData
	{
		QVector<InstanceItemPtr> m_items;

		void	serialise	(QJsonObject& json) const;
		Result	deserialise	(const QJsonObject& json, int version, Instance& instance);
	};

	class InstanceConnection : public SerializableDataWrapper<InstanceConnectionData> {};


	// Progress Item
	//--------------------------------------------------------------------------------

	struct InstanceProgressItemData
	{
		ItemCPtr m_item;
		int		 m_num;

				InstanceProgressItemData	();
		void	serialise					(QJsonObject& json) const;
		Result	deserialise					(const QJsonObject& json, int version, const ItemDatabase& item_db);
	};

	class InstanceProgressItem : public SerializableDataWrapper<InstanceProgressItemData> {};


	// Progress Dungeon
	//--------------------------------------------------------------------------------

	struct InstanceProgressLocationData
	{
		LocationCPtr	m_location;
		int				m_num_items;
		int				m_num_keys_current;
		int				m_num_keys_total;
		bool			m_has_map;
		bool			m_has_compass;
		bool			m_has_big_key;
		bool			m_is_pendant;
		bool			m_is_pendant_green;
		bool			m_is_crystal;
		bool			m_is_crystal_red;
		bool			m_cleared;

				InstanceProgressLocationData	();
		void	serialise						(QJsonObject& json) const;
		Result	deserialise						(const QJsonObject& json, int version, const LocationDatabase& location_db);
	};

	class InstanceProgressLocation : public SerializableDataWrapper<InstanceProgressLocationData> {};
}

#endif