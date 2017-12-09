// Project includes
#include "UI/MapWidget/Items/MapSceneItemBackground.h"
#include "Data/Settings.h"
#include "EditorInterface.h"

// Qt includes
#include <QPainter>


namespace LTTPMapTracker
{
	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapSceneItemBackground::MapSceneItemBackground(EditorInterface& editor_interface, QString image_id, QGraphicsItem* parent)
		: MapSceneItemPixmap(parent)
		, m_settings(editor_interface.get_settings())
	{
		QPixmap pixmap(image_id);
		setPixmap(pixmap);
		setPos(-QPointF((float)pixmap.width() * 0.5f, (float)pixmap.height() * 0.5f));
		setZValue(-1000.0f);
	}



	//================================================================================
	// QGraphicsItem Interface
	//================================================================================

	void MapSceneItemBackground::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		painter->save();
		painter->setOpacity(m_settings.get().m_map_background_opacity);
		QGraphicsPixmapItem::paint(painter, option, widget);
		painter->restore();
	}

	bool MapSceneItemBackground::contains(const QPointF& /*point*/) const
	{
		return false;
	}
}