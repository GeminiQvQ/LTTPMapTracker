#ifndef MAP_SCENE_ITEM_CONNECTION_H
#define MAP_SCENE_ITEM_CONNECTION_H

// Project includes
#include "Data/Instance/InstanceTypeInfo.h"

// Qt includes
#include <QGraphicsLineItem>

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	class EditorInterface;
	class Settings;
}


namespace LTTPMapTracker
{
	class MapSceneItemConnectionItem : public QGraphicsLineItem
	{
	public:
		// Construction & Destruction
								MapSceneItemConnectionItem	(EditorInterface& editor_interface, InstancePtr instance, InstanceConnectionPtr connection, QGraphicsItem* parent = nullptr);
								~MapSceneItemConnectionItem	();

		// QGraphicsItem Interface
		virtual void			paint						(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
		virtual void			mousePressEvent				(QGraphicsSceneMouseEvent* event) override;

		// Accessors
		InstanceConnectionPtr	get_connection				();
		InstanceConnectionCPtr	get_connection				() const;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif