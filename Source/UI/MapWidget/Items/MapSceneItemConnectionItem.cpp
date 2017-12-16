// Project includes
#include "UI/MapWidget/Items/MapSceneItemConnectionItem.h"
#include "Data/Instance/Instance.h"
#include "Data/Schema/SchemaData.h"
#include "Data/Settings.h"
#include "EditorInterface.h"

// Qt includes
#include <QApplication>
#include <QPainter>
#include <QPen>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct MapSceneItemConnectionItem::Internal
	{
		InstancePtr				m_instance;
		InstanceConnectionPtr	m_connection;

		Internal(InstancePtr instance, InstanceConnectionPtr connection)
			: m_instance(instance)
			, m_connection(connection)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapSceneItemConnectionItem::MapSceneItemConnectionItem(EditorInterface& editor_interface, InstancePtr instance, InstanceConnectionPtr connection, QGraphicsItem* parent)
		: QGraphicsLineItem(parent)
		, m_internal(std::make_unique<Internal>(instance, connection))
	{
		// Properties.
		QPen pen;
		pen.setWidthF(editor_interface.get_settings().get().m_map_connection_thickness);
		pen.setCosmetic(true);
		pen.setCapStyle(Qt::RoundCap);
		pen.setColor(editor_interface.get_settings().get().m_map_connection_color);
		setPen(pen);
		
		setBoundingRegionGranularity(0.2f);
		setZValue(-1.0f);

		// Data.
		setLine(QLineF(connection->get().m_items[0]->get().m_schema_item->get().m_position, connection->get().m_items[1]->get().m_schema_item->get().m_position));
	}

	MapSceneItemConnectionItem::~MapSceneItemConnectionItem()
	{
	}



	//================================================================================
	// QGraphicsItem Interface
	//================================================================================

	void MapSceneItemConnectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		painter->setRenderHint(QPainter::Antialiasing);
		QGraphicsLineItem::paint(painter, option, widget);
	}

	void MapSceneItemConnectionItem::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
	{
		if (QApplication::keyboardModifiers() & Qt::SHIFT)
		{
			m_internal->m_instance->connections().remove(m_internal->m_connection);
		}

		auto schema = m_internal->m_instance->get_schema();
		schema;
	}



	//================================================================================
	// Accessors
	//================================================================================

	InstanceConnectionPtr MapSceneItemConnectionItem::get_connection()
	{
		return m_internal->m_connection;
	}

	InstanceConnectionCPtr MapSceneItemConnectionItem::get_connection() const
	{
		return m_internal->m_connection;
	}
}