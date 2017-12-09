#ifndef MAP_SCENE_ITEM_INSTANCE_ITEM_H
#define MAP_SCENE_ITEM_INSTANCE_ITEM_H

// Project includes
#include "UI/MapWidget/Items/Common/MapSceneItemPixmap.h"
#include "Data/Instance/InstanceTypeInfo.h"

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	class EditorInterface;
}


namespace LTTPMapTracker
{
	class MapSceneItemInstanceItem : public QObject
								   , public MapSceneItemPixmap
	{
	public:
		// Construction & Destruction
									MapSceneItemInstanceItem		(EditorInterface& editor_interface, InstancePtr instance, InstanceItemPtr instance_item, QGraphicsItem* parent = nullptr);

		// QGraphicsItem Interface
		virtual void				paint							(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
		virtual void				mousePressEvent					(QGraphicsSceneMouseEvent* event) override;

		// Accessors
		InstanceItemPtr				get_instance_item				();
		InstanceItemCPtr			get_instance_item				() const;

	private:
		// Helpers
		void						cache							();
		void						cache_pixmap					();
		void						cache_color						();
		void						cache_tooltip					();

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif