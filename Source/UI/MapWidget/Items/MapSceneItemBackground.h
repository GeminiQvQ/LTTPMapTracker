#ifndef MAP_SCENE_ITEM_BACKGROUND_H
#define MAP_SCENE_ITEM_BACKGROUND_H

// Project includes
#include "UI/MapWidget/Items/Common/MapSceneItemPixmap.h"

// Forward declarations
namespace LTTPMapTracker
{
	class EditorInterface;
	class Settings;
}


namespace LTTPMapTracker
{
	class MapSceneItemBackground : public MapSceneItemPixmap
	{
	public:
		// Construction & Destruction
									MapSceneItemBackground	(EditorInterface& editor_interface, QString image_id, QGraphicsItem* parent = nullptr);

		// QGraphicsItem Interface
		virtual void				paint					(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
		virtual bool				contains				(const QPointF& point) const override;

	private:
		const Settings& m_settings;
	};
}

#endif