#ifndef ENTITY_WIDGET_ITEM_H
#define ENTITY_WIDGET_ITEM_H

// Project includes
#include "Data/Database/EntityDatabase.h"

// Qt includes
#include <QFont>
#include <QGraphicsLayoutItem>
#include <QGraphicsPixmapItem>
#include <QObject>
#include <QVector>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	// Types
	//--------------------------------------------------------------------------------

	struct EntityWidgetItemData
	{
		EntityCPtr	 m_entity;
		QVariantHash m_attributes;
		QString		 m_text;
		QFont		 m_font;
		bool		 m_static;
		float		 m_alpha;

		EntityWidgetItemData()
			: m_static(false)
			, m_alpha(1.0f)
		{
		}
	};

	using EntityWidgetItemDataList = QVector<EntityWidgetItemData>;


	// Entity Widget Item
	//--------------------------------------------------------------------------------

	class EntityWidgetItem : public QObject
						   , public QGraphicsPixmapItem
						   , public QGraphicsLayoutItem
	{
		Q_INTERFACES(QGraphicsLayoutItem)
		Q_OBJECT

	public:
		// Construction & Destruction
									EntityWidgetItem		(const EntityWidgetItemDataList& data_list);
									~EntityWidgetItem		();
		
		// Entity
		EntityCPtr					get_entity				()																				const;
		QVector<EntityCPtr>			get_entities			()																				const;
		
		void						set_entity_index		(int index);
		int							get_entity_index		()																				const;

		const EntityWidgetItemData&	get_data				(int index)																		const;
		void						set_data				(int index, const EntityWidgetItemData& data);

		// QGraphicsItem Interface
		virtual void				paint					(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)			override;
		virtual void				mousePressEvent			(QGraphicsSceneMouseEvent* event)														override;

		// QGraphicsLayoutItem Interface
		virtual void				setGeometry				(const QRectF& rect)																	override;
		virtual QSizeF				sizeHint				(Qt::SizeHint which, const QSizeF& constraint = QSizeF())						const	override;

	signals:
		// Signals
		void						signal_clicked			(Qt::MouseButton button);

	private:
		// Helpers
		QPixmap						create_pixmap			(const EntityWidgetItemData& data)												const;

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif