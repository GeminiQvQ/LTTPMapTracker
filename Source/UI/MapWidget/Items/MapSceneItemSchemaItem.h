#ifndef MAP_SCENE_ITEM_SCHEMA_ITEM_H
#define MAP_SCENE_ITEM_SCHEMA_ITEM_H

// Project includes
#include "UI/MapWidget/Items/Common/MapSceneItemPixmap.h"
#include "Data/Schema/SchemaTypeInfo.h"

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	class EditorInterface;
}


namespace LTTPMapTracker
{
	class MapSceneItemSchemaItem : public QObject
								 , public MapSceneItemPixmap
	{
		Q_OBJECT

	public:
		// Construction & Destruction
									MapSceneItemSchemaItem		(EditorInterface& editor_interface, SchemaItemPtr schema_item, QGraphicsItem* parent = nullptr);

		// QGraphicsItem Interface
		virtual void				paint						(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
		virtual void				mouseReleaseEvent			(QGraphicsSceneMouseEvent* event) override;

		// Accessors
		SchemaItemPtr				get_schema_item				();
		SchemaItemCPtr				get_schema_item				() const;

	private slots:
		// Schema Item Slots
		void						slot_schema_item_modified	();

	private:
		// Helpers
		void						create_pixmap				();

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif