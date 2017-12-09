#ifndef SCHEMA_H
#define SCHEMA_H

// Project includes
#include "Data/Schema/SchemaData.h"
#include "Utility/DataContainer.h"

// Qt includes
#include <QJsonObject>
#include <QObject>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	// Types
	//--------------------------------------------------------------------------------

	using SchemaItems	= SearchableDataContainer<SchemaItem, QString>;
	using SchemaRegions = SearchableDataContainer<SchemaRegion, QString>;
	using SchemaRules	= SearchableDataContainer<SchemaRule, QString>;


	// Schema
	//--------------------------------------------------------------------------------

	class Schema : public QObject
	{
		Q_OBJECT

	public:
		// Construction & Destruction
								Schema						();
								~Schema						();
				 
		// Save & Load
		Result					save						();
		Result					save						(QString filename);
		Result					load						(QString filename);

		// Properties
		QString					get_filename				();
		bool					is_dirty					() const;

		// Data
		SchemaItems&			items						();
		const SchemaItems&		items						() const;
		SchemaRegions&			regions						();
		const SchemaRegions&	regions						() const;
		SchemaRules&			rules						();
		const SchemaRules&		rules						() const;

	signals:
		// Signals
		void					signal_dirty_state_changed	(bool dirty);

	private slots:
		// Data Slots
		void					slot_region_modified		(int index);
		void					slot_rule_modified			(int index);

	private:
		// Helpers
		SchemaItemPtr			create_item					();
		SchemaRegionPtr			create_region				();
		SchemaRulePtr			create_rule					();
		void					set_dirty					();

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif