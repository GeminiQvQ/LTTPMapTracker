#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

// Project includes
#include "EditorTypeInfo.h"

// Qt includes
#include <QWidget>

// Stdlib includes
#include <memory>

// Forward declarations
class QGraphicsItem;

namespace LTTPMapTracker
{
	class EditorInterface;
	class MapScene;
	class MapSceneItem;
	struct SettingsDiff;
}


namespace LTTPMapTracker
{
	class MapWidget : public QWidget
	{
		Q_OBJECT

	public:
		// Construction & Destruction
						MapWidget					(EditorInterface& editor_interface, QWidget* parent = nullptr);
						~MapWidget					();

		// Schema
		void			set_schema					(SchemaPtr schema);
		void			clear_schema				();
		
		// Instance
		void			set_instance				(InstancePtr instance);
		void			clear_instance				();

		// Selection
		void			select_schema_item			(SchemaItemPtr schema_item);

	signals:
		// Signals
		void			signal_selection_changed	(QVector<QGraphicsItem*> items);

	protected:
		// Qt Events
		virtual bool	eventFilter					(QObject* object, QEvent* event) override;

	private slots:
		// UI Slots
		void			slot_tab_changed			(int index);
		void			slot_context_menu			();
		void			slot_add_schema_item		();
		void			slot_remove_schema_items	();
		void			slot_close_entity_widgets	();

		// Scene Slots
		void			slot_selection_changed		();

		// Settings Slots
		void			slot_settings_changed		(const SettingsDiff& diff);

	private:
		// Helpers
		void			activate_scene				(MapScene& scene);

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif