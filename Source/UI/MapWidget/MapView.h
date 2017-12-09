#ifndef MAP_VIEW_H
#define MAP_VIEW_H

// Project includes
#include "Data/Instance/InstanceTypeInfo.h"

// Qt includes
#include <QGraphicsView>

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	class MapScene;
}


namespace LTTPMapTracker
{
	class MapView : public QGraphicsView
	{
		Q_OBJECT

	public:
		// Construction & Destruction
						MapView					(QWidget* parent = nullptr);
						~MapView				();

		// Scene
		void			set_scene				(MapScene& scene, bool fit);
		MapScene*		get_scene				();
		const MapScene*	get_scene				() const;

		// Instance
		void			set_instance			(InstancePtr instance);
		void			clear_instance			();

	protected:
		// Qt Events
		virtual void	mousePressEvent			(QMouseEvent* event)	override;
		virtual void	mouseMoveEvent			(QMouseEvent* event)	override;
		virtual void	mouseReleaseEvent		(QMouseEvent* event)	override;
		virtual void	wheelEvent				(QWheelEvent* event)	override;
		virtual void	resizeEvent				(QResizeEvent* event)	override;
		virtual void	paintEvent				(QPaintEvent* event)	override;

	private:
		// Helpers
		void			clamp_zoom_levels		();
		QRectF			get_square_items_rect	() const;

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif