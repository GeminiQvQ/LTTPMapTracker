#ifndef ITEM_DATABASE_H
#define ITEM_DATABASE_H

// Project includes
#include "data/Instance/InstanceTypeInfo.h"
#include "Data/Database/EntityDatabase.h"
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
	
	struct Item;
	using ItemCPtr = std::shared_ptr<const Item>;
	using ItemList = QVector<ItemCPtr>;

	struct Item
	{
		EntityCPtr m_entity;
	};


	// Item Database
	//--------------------------------------------------------------------------------

	class ItemDatabase
	{
	public:
		// Loading
		Result		load		(const EntityDatabase& entity_db);

		// Data
		ItemCPtr	get_item	(QString type_name) const;
		ItemCPtr	get_item	(EntityCPtr entity) const;
		ItemList	get_items	()					const;

	private:
		ItemList	m_items;
	};
}

#endif