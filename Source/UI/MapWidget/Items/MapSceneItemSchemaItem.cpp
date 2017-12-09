// Project includes
#include "UI/MapWidget/Items/MapSceneItemSchemaItem.h"
#include "Data/Schema/SchemaData.h"
#include "Data/Settings.h"
#include "EditorInterface.h"

// Qt includes
#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTime>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct MapSceneItemSchemaItem::Internal
	{
		EditorInterface& m_editor_interface;
		SchemaItemPtr	 m_schema_item;
		bool			 m_schema_item_sync;
		float			 m_selection_dash_offset;

		Internal(EditorInterface& editor_interface, SchemaItemPtr schema_item)
			: m_editor_interface(editor_interface)
			, m_schema_item(schema_item)
			, m_schema_item_sync(false)
			, m_selection_dash_offset(0.0f)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapSceneItemSchemaItem::MapSceneItemSchemaItem(EditorInterface& editor_interface, SchemaItemPtr schema_item, QGraphicsItem* parent)
		: MapSceneItemPixmap(parent)
		, m_internal(std::make_unique<Internal>(editor_interface, schema_item))
	{
		// Create pixmap.
		create_pixmap();

		// Properties.
		setPos(schema_item->get().m_position);
		setOffset(-pixmap().width() * 0.5f, -pixmap().height() * 0.5f);
		setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIgnoresTransformations | QGraphicsItem::ItemSendsGeometryChanges);
		setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

		// Signals.
		connect(schema_item.get(), &SchemaItem::signal_modified, this, &MapSceneItemSchemaItem::slot_schema_item_modified);
	}



	//================================================================================
	// QGraphicsItem Interface
	//================================================================================

	void MapSceneItemSchemaItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		auto option_internal = *option;
		option_internal.state &= ~QStyle::State_Selected;

		if (isSelected())
		{
			m_internal->m_selection_dash_offset -= 0.01f;

			QPen pen;
			pen.setColor(QColor(255, 255, 0));
			pen.setDashOffset(m_internal->m_selection_dash_offset);
			pen.setDashPattern(QVector<qreal>() << 3.0f << 2.0f);

			painter->save();
			painter->setPen(pen);
			painter->drawRect(boundingRect().adjusted(0.0f, 0.0f, 1.0f, 1.0f));
			painter->restore();

			update();
		}

		QGraphicsPixmapItem::paint(painter, &option_internal, widget);
	}

	void MapSceneItemSchemaItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
	{
		MapSceneItemPixmap::mouseReleaseEvent(event);

		if (!m_internal->m_schema_item_sync && m_internal->m_schema_item->get().m_position != pos())
		{
			auto data = m_internal->m_schema_item->get();
			data.m_position = pos();
			m_internal->m_schema_item->set(data);
		}
	}



	//================================================================================
	// Accessors
	//================================================================================

	SchemaItemPtr MapSceneItemSchemaItem::get_schema_item()
	{
		return m_internal->m_schema_item;
	}

	SchemaItemCPtr MapSceneItemSchemaItem::get_schema_item() const
	{
		return m_internal->m_schema_item;
	}



	//================================================================================
	// Schema Item Slots
	//================================================================================

	void MapSceneItemSchemaItem::slot_schema_item_modified()
	{
		m_internal->m_schema_item_sync = true;
		create_pixmap();
		setPos(m_internal->m_schema_item->get().m_position);
		m_internal->m_schema_item_sync = false;
	}



	//================================================================================
	// Helpers
	//================================================================================

	void MapSceneItemSchemaItem::create_pixmap()
	{
		int size = m_internal->m_editor_interface.get_settings().get().m_map_item_size;
		int border_size = (float)size * 0.125f;
		QRect rect(0, 0, size, size);
		
		QFont font;
		font.setBold(true);
		font.setPixelSize((float)size * 0.8f);

		QColor color(200, 200, 200);
		auto region = m_internal->m_schema_item->get().m_region;
		if (region != nullptr)
		{
			color = region->get().m_color;
		}

		QPixmap pixmap(size, size);
		QPainter painter(&pixmap);
		painter.setFont(font);
		painter.setBrush(color.lighter());
		painter.drawRect(rect);
		painter.setBrush(color);
		painter.drawRect(rect.adjusted(border_size, border_size, -border_size, -border_size));
		painter.setBrush(QColor(0, 0, 0));
		painter.drawText(rect.adjusted(1, 0, 0, 0), "?", QTextOption(Qt::AlignCenter));
		setPixmap(pixmap);
	}
}