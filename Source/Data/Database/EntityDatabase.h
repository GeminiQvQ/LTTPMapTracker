#ifndef ENTITY_DATABASE_H
#define ENTITY_DATABASE_H

// Project includes
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
	
	struct Entity;
	using EntityCPtr = std::shared_ptr<const Entity>;
	using EntityList = QVector<EntityCPtr>;

	struct Entity
	{
		QString	m_type_name;
		QString	m_display_name;
		QPixmap	m_image;
	};


	// Entity Database
	//--------------------------------------------------------------------------------

	class EntityDatabase
	{
	public:
		// Loading
		Result		load			();

		// Entity
		EntityCPtr	get_entity		(QString type_name) const;
		EntityList	get_entities	()					const;

	private:
		EntityList	m_entities;
	};
}

#endif