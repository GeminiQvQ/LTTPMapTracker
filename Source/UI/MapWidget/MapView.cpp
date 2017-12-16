// Project includes
#include "UI/MapWidget/MapView.h"
#include "UI/MapWidget/MapScene.h"
#include "UI/MapWidget/Items/MapSceneItemInstanceItem.h"
#include "Data/Instance/Instance.h"

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QWheelEvent>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct MapView::Internal
	{
		QPoint						m_mouse_origin;
		QPoint						m_mouse_current;

		bool						m_panning;
		MapSceneItemInstanceItem*	m_connecting_item;

		InstancePtr					m_instance;

		Internal()
			: m_panning(false)
			, m_connecting_item(nullptr)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapView::MapView(QWidget* parent)
		: QGraphicsView(parent)
		, m_internal(std::make_unique<Internal>())
	{
		// Widget.
		setTransformationAnchor(QGraphicsView::NoAnchor);
	}

	MapView::~MapView()
	{
	}



	//================================================================================
	// Scene
	//================================================================================

	void MapView::set_scene(MapScene& scene, bool fit)
	{
		setScene(&scene);

		if (fit)
		{
			fitInView(get_square_items_rect());	
		}
	}

	MapScene* MapView::get_scene()
	{
		return static_cast<MapScene*>(scene());
	}

	const MapScene* MapView::get_scene() const
	{
		return static_cast<const MapScene*>(scene());
	}



	//================================================================================
	// Instance
	//================================================================================

	void MapView::set_instance(InstancePtr instance)
	{
		clear_instance();

		QObject::connect(instance.get(), &Instance::signal_accessibility_cached, this, (void(MapView::*)())&MapView::update);
		m_internal->m_instance = instance;
	}

	void MapView::clear_instance()
	{
		if (m_internal->m_instance != nullptr)
		{
			m_internal->m_instance->disconnect(this);
			m_internal->m_instance = nullptr;
		}
	}



	//================================================================================
	// Qt Events
	//================================================================================

	void MapView::mousePressEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton)
		{
			m_internal->m_mouse_origin = event->pos();
			m_internal->m_mouse_current = event->pos();

			auto item = scene()->itemAt(mapToScene(event->pos()), transform());
			if (item == nullptr)
			{
				m_internal->m_panning = true;
			}
			else if (get_scene()->get_item_type(*item) == MapSceneItemType::InstanceItem)
			{
				m_internal->m_connecting_item = static_cast<MapSceneItemInstanceItem*>(item);
				return;
			}
		}

		QGraphicsView::mousePressEvent(event);

		update();
	}

	void MapView::mouseReleaseEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton)
		{
			auto delta = (event->pos() - m_internal->m_mouse_origin).manhattanLength();
			bool is_drag = (delta >= 2);

			if (m_internal->m_connecting_item != nullptr)
			{
				if (is_drag)
				{
					auto item = scene()->itemAt(mapToScene(event->pos()), transform());
					if (item != nullptr && get_scene()->get_item_type(*item) == MapSceneItemType::InstanceItem)
					{
						auto instance_item_a = m_internal->m_connecting_item->get_instance_item();
						auto instance_item_b = static_cast<MapSceneItemInstanceItem*>(item)->get_instance_item();

						if (instance_item_a != instance_item_b)
						{
							auto connections = m_internal->m_instance->connections().get();
							if (std::none_of(connections.begin(), connections.end(), [instance_item_a, instance_item_b] (InstanceConnectionCPtr connection)
							{
								return (connection->get().m_items.contains(instance_item_a) &&
										connection->get().m_items.contains(instance_item_b));
							}))
							{
								m_internal->m_instance->connections().add({instance_item_a, instance_item_b});
							}
						}
					}
				}
				else
				{
					QGraphicsView::mousePressEvent(event);
				}
				
				m_internal->m_connecting_item = nullptr;
			}

			m_internal->m_panning = false;

			m_internal->m_mouse_origin = QPoint();
			m_internal->m_mouse_current = QPoint();
		}

		QGraphicsView::mouseReleaseEvent(event);

		update();
	}

	void MapView::mouseMoveEvent(QMouseEvent* event)
	{
		auto scene_delta = mapToScene(event->pos()) - mapToScene(m_internal->m_mouse_current);
		m_internal->m_mouse_current = event->pos();

		if (m_internal->m_panning)
		{
			translate(scene_delta.x(), scene_delta.y());
			return;
		}

		QGraphicsView::mouseMoveEvent(event);
		
		update();
	}

	void MapView::wheelEvent(QWheelEvent* event)
	{
		// Perform normal zoom.
		auto view_pos = event->pos();
		auto scene_pos = mapToScene(view_pos);

		float factor = 1.0f;
		float angle = event->angleDelta().y();
		
		if (angle > 0)
		{
			factor = 1.0f + (qMin(angle, 360.0f) / 360.0f * 0.5f);
		}

		if (angle < 0)
		{
			factor = 1.0f - (-qMax(angle, -360.0f) / 360.0f * 0.5f);
		}

		scale(factor, factor);

		// Ensure the current rect is visible in order to zoom around the cursor point.
		auto view_rect = viewport()->rect();
		auto scene_rect = mapToScene(view_rect).boundingRect();
		auto offset_pos = QPointF(scene_pos.x() - view_pos.x() * scene_rect.width() / view_rect.width(), scene_pos.y() - view_pos.y() * scene_rect.height() / view_rect.height());
		ensureVisible(offset_pos.x(), offset_pos.y(), scene_rect.width(), scene_rect.height(), 0, 0);
		
		// Attempt to correct some drifting.
		auto scene_pos_new = mapToScene(view_pos);
		ensureVisible(QRectF(offset_pos - scene_pos_new + scene_pos, scene_rect.size()), 0, 0);
		
		// Clamp outer zoom.
		clamp_zoom_levels();
	}

	void MapView::resizeEvent(QResizeEvent* /*event*/)
	{
		clamp_zoom_levels();
	}

	void MapView::paintEvent(QPaintEvent* event)
	{
		QGraphicsView::paintEvent(event);

		if (m_internal->m_connecting_item != nullptr)
		{
			auto pos_a = mapFromScene(m_internal->m_connecting_item->pos());
			auto pos_b = m_internal->m_mouse_current;

			QPainter painter(viewport());

			QPen pen;
			pen.setColor(QColor(255, 255, 0));
			pen.setDashPattern(QVector<qreal>() << 3.0f << 2.0f);

			painter.setPen(pen);
			painter.drawLine(pos_a, pos_b);
		}
	}



	//================================================================================
	// Helpers
	//================================================================================

	void MapView::clamp_zoom_levels()
	{
		auto view_rect = QRectF(viewport()->rect());
		auto scene_rect_new = mapToScene(view_rect.toAlignedRect()).boundingRect();
		auto items_rect = get_square_items_rect();

		if (scene_rect_new.left() < items_rect.left() ||
			scene_rect_new.top() < items_rect.top()  ||
			scene_rect_new.right() > items_rect.right()  ||
			scene_rect_new.bottom() > items_rect.bottom())
		{
			auto max_rect = QRectF(QPointF(qMax(scene_rect_new.left(), items_rect.left()), qMax(scene_rect_new.top(), items_rect.top())),
								   QPointF(qMin(scene_rect_new.right(), items_rect.right()), qMin(scene_rect_new.bottom(), items_rect.bottom())));

			fitInView(max_rect);
		}
	}

	QRectF MapView::get_square_items_rect() const
	{
		auto view_rect = QRectF(viewport()->rect());
		auto items_rect = scene()->itemsBoundingRect();

		qreal x_ratio = (view_rect.width() < view_rect.height() ? view_rect.height() / view_rect.width() : 1.0f);
		qreal y_ratio = (view_rect.height() < view_rect.width() ? view_rect.width() / view_rect.height() : 1.0f);

		return QRectF(QPointF(items_rect.left() * y_ratio, items_rect.top() * x_ratio), 
					  QPointF(items_rect.right() * y_ratio, items_rect.bottom() * x_ratio));
	}
}