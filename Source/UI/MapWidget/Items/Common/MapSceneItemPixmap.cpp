// Project includes
#include "UI/MapWidget/Items/Common/MapSceneItemPixmap.h"

// Qt includes
#include <QApplication>
#include <QGraphicsScene>


namespace LTTPMapTracker
{
	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapSceneItemPixmap::MapSceneItemPixmap(QGraphicsItem* parent)
		: QGraphicsPixmapItem(parent)
	{
	}



	//================================================================================
	// QGraphicsItem Interface
	//================================================================================

	QVariant MapSceneItemPixmap::itemChange(GraphicsItemChange change, const QVariant& value)
	{
		if (scene() == nullptr)
		{
			return value;
		}

		// Clamp to scene rect.
		if (change == QGraphicsItem::ItemPositionChange)
		{
			auto item_pos = value.toPointF();
			auto item_rect = boundingRect().translated(item_pos);
			auto scene_rect = scene()->sceneRect();

			if (item_rect.left() < scene_rect.left())
			{
				item_pos.setX(scene_rect.left() + item_rect.width() * 0.5f);
			}

			if (item_rect.right() > scene_rect.right())
			{
				item_pos.setX(scene_rect.right() - item_rect.width() * 0.5f);
			}

			if (item_rect.top() < scene_rect.top())
			{
				item_pos.setY(scene_rect.top() + item_rect.height() * 0.5f);
			}

			if (item_rect.bottom() > scene_rect.bottom())
			{
				item_pos.setY(scene_rect.bottom() - item_rect.height() * 0.5f);
			}

			return item_pos;
		}
		
		// Maintain Z values based on selection order.
		if (change == QGraphicsItem::ItemSelectedChange)
		{
			if (value.toBool())
			{
				auto items = scene()->items();
				
				items.erase(std::remove_if(items.begin(), items.end(), [] (QGraphicsItem* item)
				{
					return (item->zValue() < 0.0f);
				}));

				qSort(items.begin(), items.end(), [] (QGraphicsItem* a, QGraphicsItem* b)
				{
					return (a->zValue() < b->zValue());
				});

				items.removeOne(this);
				items.push_back(this);

				for (int i = 0; i < items.size(); ++i)
				{
					items[i]->setZValue(i);
				}
			}
		}

		return value;
	}
}