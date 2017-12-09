#ifndef DATA_MODEL_H
#define DATA_MODEL_H

// Project includes
#include "Utility/Result.h"

// Qt includes
#include <QMap>

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	class EntityDatabase;
	class ItemDatabase;
	class LocationDatabase;
}


namespace LTTPMapTracker
{
	class DataModel
	{
	public:
		// Construction & Destruction
								DataModel		();
								~DataModel		();

		// Loading
		QMap<QString, Result>	load			();

		// Database
		const EntityDatabase&	get_entity_db	() const;
		const ItemDatabase&		get_item_db		() const;
		const LocationDatabase& get_location_db	() const;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif