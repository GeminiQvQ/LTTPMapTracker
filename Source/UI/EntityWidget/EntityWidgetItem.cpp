// Project includes
#include "UI/EntityWidget/EntityWidgetItem.h"

// Qt includes
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOption>
#include <QVariant>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct EntityWidgetItem::Internal
	{
		struct Entry
		{
			EntityWidgetItemData m_data;
			QPixmap				 m_pixmap;
		};

		QVector<Entry>	m_entries;
		int				m_index;

		Internal()
			: m_index(0)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	EntityWidgetItem::EntityWidgetItem(const EntityWidgetItemDataList& data_list)
		: m_internal(std::make_unique<Internal>())
	{
		for (auto data : data_list)
		{
			Internal::Entry entry;
			entry.m_data = data;
			entry.m_pixmap = create_pixmap(data);
			m_internal->m_entries << entry;
		}

		setAcceptHoverEvents(true);
		setGraphicsItem(this);
		setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
		set_entity_index(0);
	}

	EntityWidgetItem::~EntityWidgetItem()
	{
	}



	//================================================================================
	// Entity
	//================================================================================

	EntityCPtr EntityWidgetItem::get_entity() const
	{
		return m_internal->m_entries[m_internal->m_index].m_data.m_entity;
	}

	QVector<EntityCPtr> EntityWidgetItem::get_entities() const
	{
		QVector<EntityCPtr> entities;
		for (auto& entry : m_internal->m_entries)
		{
			entities << entry.m_data.m_entity;
		}
		return entities;
	}

	void EntityWidgetItem::set_entity_index(int index)
	{
		if (index < 0)
		{
			index = m_internal->m_entries.size() - 1;
		}

		if (index >= m_internal->m_entries.size())
		{
			index = 0;
		}

		m_internal->m_index = index;

		setPixmap(m_internal->m_entries[index].m_pixmap);
	}

	int EntityWidgetItem::get_entity_index() const
	{
		return m_internal->m_index;
	}

	const LTTPMapTracker::EntityWidgetItemData& EntityWidgetItem::get_data(int index) const
	{
		return m_internal->m_entries[index].m_data;
	}

	void EntityWidgetItem::set_data(int index, const EntityWidgetItemData& data)
	{
		m_internal->m_entries[index].m_data = data;
		m_internal->m_entries[index].m_pixmap = create_pixmap(data);
		update();
	}



	//================================================================================
	// QGraphicsItem Interface
	//================================================================================

	void EntityWidgetItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
	{
		auto& entry = m_internal->m_entries[m_internal->m_index];
	
		if ((option->state & QStyle::State_MouseOver) && !entry.m_data.m_static)
		{
			auto pixmap = this->pixmap();

			QPainter pixmap_painter(&pixmap);
			pixmap_painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
			pixmap_painter.fillRect(pixmap.rect(), QColor(255, 255, 0, 128));

			painter->drawPixmap(pixmap.rect(), pixmap);

			return;
		}

		painter->setOpacity(entry.m_data.m_alpha);

		QGraphicsPixmapItem::paint(painter, option, widget);
	}

	void EntityWidgetItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		auto& entry = m_internal->m_entries[m_internal->m_index];
		if (!entry.m_data.m_static)
		{
			emit signal_clicked(event->button());
		}
	}



	//================================================================================
	// QGraphicsLayoutItem Interface
	//================================================================================

	void EntityWidgetItem::setGeometry(const QRectF& rect)
	{
		setPos(rect.topLeft());
	}

	QSizeF EntityWidgetItem::sizeHint(Qt::SizeHint /*which*/, const QSizeF& /*constraint*/) const
	{
		return pixmap().size();
	}



	//================================================================================
	// Helpers
	//================================================================================

	QPixmap EntityWidgetItem::create_pixmap(const EntityWidgetItemData& data) const
	{
		auto pixmap = data.m_entity->m_image;

		if (!data.m_text.isEmpty())
		{
			auto text_rect = QFontMetrics(data.m_font).boundingRect(data.m_text).adjusted(-1, 0, 0, 0);
			QPixmap text_pixmap(text_rect.size());
			QPixmap box_pixmap(text_rect.size() + QSize(4, 4));
			
			QPainter painter;
			
			painter.begin(&text_pixmap);
			painter.setBrush(QColor(0, 0, 0));
			painter.setFont(data.m_font);
			painter.drawRect(text_pixmap.rect());
			painter.setPen(QColor(255, 255, 255));
			painter.drawText(QPoint(0, -text_rect.y()), data.m_text);
			painter.end();

			painter.begin(&box_pixmap);
			painter.setBrush(QColor(0, 0, 0));
			painter.setPen(QColor(255, 255, 255));
			painter.drawRect(box_pixmap.rect().adjusted(0, 0, -1, -1));
			painter.drawPixmap(QPoint((box_pixmap.width() - text_pixmap.width()) / 2, (box_pixmap.height() - text_pixmap.height()) / 2), text_pixmap);
			painter.end();

			painter.begin(&pixmap);
			painter.translate(pixmap.rect().bottomRight() - box_pixmap.rect().bottomRight());
			painter.drawPixmap(QPoint(0, 0), box_pixmap);
			painter.end();
		}

		return pixmap;
	}
}