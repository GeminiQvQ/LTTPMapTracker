// Project includes
#include "UI/MapWidget/Items/MapSceneItemInstanceItem.h"
#include "Data/Database/EntityDatabase.h"
#include "Data/Instance/Instance.h"
#include "Data/Instance/InstanceRuleParser.h"
#include "Data/Schema/SchemaData.h"
#include "Data/DataModel.h"
#include "Data/Settings.h"
#include "EditorInterface.h"

// Qt includes
#include <QFont>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOption>
#include <QTime>
#include <QToolTip>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct MapSceneItemInstanceItem::Internal
	{
		EditorInterface& m_editor_interface;

		InstancePtr		 m_instance;
		InstanceItemPtr	 m_instance_item;

		QColor			 m_color;

		Internal(EditorInterface& editor_interface, InstancePtr instance, InstanceItemPtr instance_item)
			: m_editor_interface(editor_interface)
			, m_instance(instance)
			, m_instance_item(instance_item)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapSceneItemInstanceItem::MapSceneItemInstanceItem(EditorInterface& editor_interface, InstancePtr instance, InstanceItemPtr instance_item, QGraphicsItem* parent)
		: MapSceneItemPixmap(parent)
		, m_internal(std::make_unique<Internal>(editor_interface, instance, instance_item))
	{
		// Cache initial data.
		cache();

		// Properties.
		setPos(instance_item->get().m_schema_item->get().m_position);
		setOffset(-pixmap().width() * 0.5f, -pixmap().height() * 0.5f);
		setFlags(QGraphicsItem::ItemIgnoresTransformations);
		setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

		// Signals.
		for (auto item : instance->items())
		{
			connect(item.get(), &InstanceItem::signal_modified, this, &MapSceneItemInstanceItem::cache);
		}
		
		connect(instance.get(), &Instance::signal_accessibility_cached, this, &MapSceneItemInstanceItem::cache);
	}



	//================================================================================
	// QGraphicsItem Interface
	//================================================================================

	void MapSceneItemInstanceItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		auto opacity = m_internal->m_color.alphaF();
		painter->setOpacity(opacity);

		QGraphicsPixmapItem::paint(painter, option, widget);
	}

	void MapSceneItemInstanceItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton)
		{
			auto data = m_internal->m_instance_item->get();

			if (!data.m_cleared)
			{
				data.m_cleared = true;
				m_internal->m_instance_item->set(data);
			}

			if (data.m_cleared && event->modifiers() & Qt::ShiftModifier)
			{
				data.m_cleared = false;
				m_internal->m_instance_item->set(data);
			}
		}
	}



	//================================================================================
	// Accessors
	//================================================================================

	InstanceItemPtr MapSceneItemInstanceItem::get_instance_item()
	{
		return m_internal->m_instance_item;
	}

	InstanceItemCPtr MapSceneItemInstanceItem::get_instance_item() const
	{
		return m_internal->m_instance_item;
	}



	//================================================================================
	// Helpers
	//================================================================================

	void MapSceneItemInstanceItem::cache()
	{
		cache_color();
		cache_pixmap();
		cache_tooltip();

		setZValue(!m_internal->m_instance_item->get().m_cleared ? 1.0f : 0.0f);
	}

	void MapSceneItemInstanceItem::cache_pixmap()
	{
		auto& data = m_internal->m_instance_item->get();

		auto color = m_internal->m_color;
		color.setAlpha(255);

		int size = m_internal->m_editor_interface.get_settings().get().m_map_item_size;
		int border_size = (float)size * 0.125f;
		auto rect = QRect(0, 0, size, size);

		QPixmap pixmap(size, size);
		QPainter painter(&pixmap);

		painter.setBrush(color.lighter());
		painter.drawRect(rect);
		painter.setBrush(color);
		painter.drawRect(rect.adjusted(border_size, border_size, -border_size, -border_size));

		if (data.m_location != nullptr)
		{
			painter.drawPixmap(rect.adjusted(border_size, border_size, -border_size, -border_size), data.m_location->m_entity->m_image);

			if (data.m_location_entrance != nullptr)
			{
				auto entrance_pixmap = data.m_location_entrance->m_image;
				painter.drawPixmap(rect.bottomRight() - QPoint(pixmap.width() - 1, entrance_pixmap.height() - 1), entrance_pixmap);
			}
		}
		else
		{
			QFont font;
			font.setBold(true);
			font.setPixelSize((float)size * 0.8f);
			painter.setFont(font);
			painter.setBrush(QColor(0, 0, 0));
			painter.drawText(rect.adjusted(1, 0, 0, 0), "?", QTextOption(Qt::AlignCenter));
		}

		if (!data.m_items.isEmpty())
		{
			auto entity = m_internal->m_editor_interface.get_data_model().get_entity_db().get_entity(m_internal->m_editor_interface.get_settings().get().m_map_item_entity_item_requirement);
			if (entity != nullptr)
			{
				painter.drawPixmap(rect.bottomLeft() - QPoint(0, entity->m_image.height() - 1), entity->m_image);
			}
		}

		setPixmap(pixmap);
	}

	void MapSceneItemInstanceItem::cache_color()
	{
		auto& settings = m_internal->m_editor_interface.get_settings().get();
		auto color = settings.m_map_item_color_base;

		auto& data = m_internal->m_instance_item->get();

		if (!data.m_accessible)
		{
			color = settings.m_map_item_color_inaccessible;
		}
		else
		{
			bool requires_items = false;

			if (!data.m_items.isEmpty())
			{
				requires_items = std::any_of(data.m_items.begin(), data.m_items.end(), [this] (ItemCPtr item)
				{
					return !m_internal->m_instance->progress_items().contains(item->m_entity);
				});

				if (requires_items)
				{
					color = settings.m_map_item_color_item_requirement;
				}
				else
				{
					color = settings.m_map_item_color_item_requirement_fulfilled;
				}
			}
			
			if (!requires_items && data.m_location != nullptr)
			{
				auto match_result = match_location_requirements(data.m_location->m_requirements, *m_internal->m_instance);

				switch (match_result)
				{
				case LocationMatch::No: color = settings.m_map_item_color_location_requirement; break;
				case LocationMatch::Maybe: color = settings.m_map_item_color_location; break;
				case LocationMatch::Yes: color = settings.m_map_item_color_location_requirement_fulfilled; break;
				}
			}
		}

		if (data.m_cleared)
		{
			color.setAlphaF(settings.m_map_item_opacity_cleared);
		}

		m_internal->m_color = color;
	}

	void MapSceneItemInstanceItem::cache_tooltip()
	{
		QString tooltip;

		auto& data = m_internal->m_instance_item->get();
		if (!data.m_items.isEmpty())
		{
			tooltip = "<h3>Required Items</h3>";
			for (auto item : data.m_items)
			{
				tooltip += QString("<font color=%1>%2</font><br>").arg(!m_internal->m_instance->progress_items().contains(item->m_entity) ? "#FF9090" : "#00FF00").arg(item->m_entity->m_display_name);
			}
			tooltip.chop(4);
		}

		setToolTip(tooltip);
	}
}