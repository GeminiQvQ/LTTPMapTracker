#ifndef SCHEMA_REGION_WIDGET_H
#define SCHEMA_REGION_WIDGET_H

// Project includes
#include "EditorTypeInfo.h"

// Qt includes
#include <QWidget>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class SchemaRegionWidget : public QWidget
	{
		Q_OBJECT

	public:
		// Construction & Destruction
				SchemaRegionWidget			(const EditorInterface& editor_interface, QWidget* parent = nullptr);
				~SchemaRegionWidget			();

		// Schema
		void	set_schema					(SchemaPtr schema);
		void	clear_schema				();

		// Selection
		void	select_region				(SchemaRegionPtr region);

	private slots:
		// UI Slots
		void	slot_list_menu				();
		void	slot_list_selection_changed	();

		void	slot_add_region				();
		void	slot_remove_region			();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif