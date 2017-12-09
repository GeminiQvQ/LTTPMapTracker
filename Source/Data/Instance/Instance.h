#ifndef INSTANCE_H
#define INSTANCE_H

// Project includes
#include "Data/Instance/InstanceData.h"
#include "Utility/DataContainer.h"
#include "EditorTypeInfo.h"

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	// Types
	//--------------------------------------------------------------------------------

	using InstanceConnections = SearchableDataContainer<InstanceConnection, InstanceItemPtr, QVector<InstanceItemPtr>>;
	using InstanceProgressItems = SearchableDataContainer<InstanceProgressItem, EntityCPtr, ItemCPtr>;
	using InstanceProgressLocations = SearchableDataContainer<InstanceProgressLocation, EntityCPtr, LocationCPtr>;


	// Instance
	//--------------------------------------------------------------------------------

	class Instance : public QObject
	{
		Q_OBJECT

	public:
		// Construction & Destruction
											Instance						(const DataModel& data_model, SchemaCPtr schema);
											~Instance						();

		// Save & Load
		Result								save							();
		Result								save							(QString filename);
		Result								save_auto						();
		Result								load							(QString filename);
		Result								load_template					(QString filename);

		// Properties
		QString								get_filename					();
		bool								is_dirty						() const;

		// Data
		const QVector<InstanceItemPtr>&		items							();
		const QVector<InstanceItemCPtr>&	items							() const;

		InstanceConnections&				connections						();
		const InstanceConnections&			connections						() const;

		InstanceProgressItems&				progress_items					();
		const InstanceProgressItems&		progress_items					() const;

		InstanceProgressLocations&			progress_locations				();
		const InstanceProgressLocations&	progress_locations				() const;

		// Accessors
		SchemaCPtr							get_schema						() const;

	signals:
		// Signals
		void								signal_dirty_state_changed		(bool dirty);
		void								signal_accessibility_cached		();

	private:
		// Helpers
		InstanceItemPtr						create_item						(SchemaItemCPtr schema_item);
		InstanceConnectionPtr				create_connection_empty			();
		InstanceConnectionPtr				create_connection				(QVector<InstanceItemPtr> instance_items);
		InstanceProgressItemPtr				create_progress_item_empty		();
		InstanceProgressItemPtr				create_progress_item			(ItemCPtr item);
		InstanceProgressLocationPtr			create_progress_location_empty	();
		InstanceProgressLocationPtr			create_progress_location		(LocationCPtr location);

		void								set_dirty						();
		void								cache_accessibility				();

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif