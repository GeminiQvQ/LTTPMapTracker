#ifndef LOCATION_DATABASE_H
#define LOCATION_DATABASE_H

// Project includes
#include "Data/Database/EntityDatabase.h"
#include "Data/Instance/InstanceTypeInfo.h"
#include "Utility/Result.h"

// Qt includes
#include <QPixmap>
#include <QString>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	// Types
	//--------------------------------------------------------------------------------
	
	struct Location;
	using LocationCPtr = std::shared_ptr<const Location>;
	using LocationList = QVector<LocationCPtr>;

	enum class LocationRequirementType
	{
		ProgressItem,
		ProgressLocation,
		ProgressSpecial
	};

	struct LocationRequirementEntry
	{
		LocationRequirementType	m_type;
		QVariant				m_value;
		bool					m_optional;
	};

	struct LocationRequirement
	{
		QVector<LocationRequirementEntry> m_entries;
	};

	struct LocationConnection
	{
		EntityList						m_entrances;
		QVector<LocationRequirement>	m_requirements;
	};

	enum class LocationMatch
	{
		No,
		Maybe,
		Yes
	};

	struct Location
	{
		EntityCPtr						m_entity;
		int								m_treasure;
		int								m_treasure_keys;
		int								m_treasure_etc;
		bool							m_is_startpos;
		EntityList						m_entrances;
		QVector<LocationRequirement>	m_requirements;
		QVector<LocationConnection>		m_connections;

		Location();
	};


	// Location Database
	//--------------------------------------------------------------------------------

	class LocationDatabase
	{
	public:
		// Loading
		Result			load			(const EntityDatabase& entity_db);

		// Entity
		LocationCPtr	get_location	(QString type_name) const;
		LocationCPtr	get_location	(EntityCPtr entity) const;
		LocationList	get_locations	()					const;

	private:
		LocationList	m_locations;
	};


	// Utility
	//--------------------------------------------------------------------------------

	Result			deserialise_location_requirements	(QVector<LocationRequirement>& requirements, const QJsonArray& json, const EntityDatabase& entity_db);
	LocationMatch	match_location_requirement			(const LocationRequirement& requirement, const Instance& instance);
	LocationMatch	match_location_requirements			(const QVector<LocationRequirement>& requirements, const Instance& instance);
}

#endif