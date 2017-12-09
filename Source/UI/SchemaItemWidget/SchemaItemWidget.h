#ifndef SCHEMA_ITEM_WIDGET_H
#define SCHEMA_ITEM_WIDGET_H

// Project includes
#include "EditorTypeInfo.h"

// Qt includes
#include <QWidget>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class SchemaItemWidget : public QWidget
	{
		Q_OBJECT

	public:
		// Construction & Destruction
				SchemaItemWidget			(QWidget* parent = nullptr);
				~SchemaItemWidget			();

		// Schema
		void	set_schema					(SchemaPtr schema);
		void	clear_schema				();

		// Selection
		void	select_item					(SchemaItemPtr item);

	signals:
		// Signals
		void	signal_selection_changed	(SchemaItemPtr item);

	private slots:
		// UI Slots
		void	slot_list_menu				();
		void	slot_list_selection_changed	();

		void	slot_add_item				();
		void	slot_remove_item			();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif