#ifndef MAP_SCENE_ITEM_PIXMAP_H
#define MAP_SCENE_ITEM_PIXMAP_H

// Qt includes
#include <QGraphicsPixmapItem>


namespace LTTPMapTracker
{
	class MapSceneItemPixmap : public QGraphicsPixmapItem
	{
	public:
		// Construction & Destruction
							MapSceneItemPixmap	(QGraphicsItem* parent = nullptr);

	protected:
		// QGraphicsItem Interface
		virtual QVariant	itemChange			(GraphicsItemChange change, const QVariant& value) override;
	};
}

#endif