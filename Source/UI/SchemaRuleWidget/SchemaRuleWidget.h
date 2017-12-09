#ifndef SCHEMA_RULE_WIDGET_H
#define SCHEMA_RULE_WIDGET_H

// Project includes
#include "EditorTypeInfo.h"

// Qt includes
#include <QWidget>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class SchemaRuleWidget : public QWidget
	{
		Q_OBJECT

	public:
		// Construction & Destruction
				SchemaRuleWidget				(EditorInterface& editor_interface, QWidget* parent = nullptr);
				~SchemaRuleWidget				();

		// Schema
		void	set_schema						(SchemaPtr schema);
		void	clear_schema					();

		// Selection
		void	select_rule						(SchemaRulePtr rule);

	private slots:
		// UI Slots
		void	slot_list_menu					();
		void	slot_list_selection_changed		();
		void	slot_properties_menu			();

		void	slot_add_rule					();
		void	slot_remove_rule				();

		void	slot_add_rule_entry				();
		void	slot_insert_rule_entry_before	();
		void	slot_insert_rule_entry_after	();
		void	slot_remove_rule_entry			();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif