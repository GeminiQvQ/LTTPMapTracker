// Project includes
#include "Data/DataModel.h"
#include "Data/Database/EntityDatabase.h"
#include "Data/Database/ItemDatabase.h"
#include "Data/Database/LocationDatabase.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct DataModel::Internal
	{
		EntityDatabase	 m_entity_db;
		ItemDatabase	 m_item_db;
		LocationDatabase m_location_db;
	};
	


	//================================================================================
	// Construction & Destruction
	//================================================================================

	DataModel::DataModel()
		: m_internal(std::make_unique<Internal>())
	{
	}

	DataModel::~DataModel()
	{
	}



	//================================================================================
	// Loading
	//================================================================================

	QMap<QString, Result> DataModel::load()
	{
		QMap<QString, Result> result;
		result["Entity Database"] = m_internal->m_entity_db.load();
		result["Item Database"] = m_internal->m_item_db.load(m_internal->m_entity_db);
		result["Location Database"] = m_internal->m_location_db.load(m_internal->m_entity_db);
		return result;
	}



	//================================================================================
	// Database
	//================================================================================

	const EntityDatabase& DataModel::get_entity_db() const
	{
		return m_internal->m_entity_db;
	}

	const ItemDatabase& DataModel::get_item_db() const
	{
		return m_internal->m_item_db;
	}

	const LocationDatabase& DataModel::get_location_db() const
	{
		return m_internal->m_location_db;
	}
}