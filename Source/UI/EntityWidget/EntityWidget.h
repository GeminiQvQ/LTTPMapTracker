#ifndef ENTITY_WIDGET_H
#define ENTITY_WIDGET_H

// Project includes
#include "Utility/Result.h"

// Qt includes
#include <QWidget>

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	class EntityDatabase;
	class EntityWidgetItem;
	struct EntityWidgetItemData;
} 


namespace LTTPMapTracker
{
	class EntityWidget : public QWidget
	{
		Q_OBJECT

	public:
		// Construction & Destruction
											EntityWidget	(const EntityDatabase& entity_db, Qt::Alignment alignment, QWidget* parent = nullptr);
											~EntityWidget	();

		// Loading
		virtual Result						load			(QString filename);

		// Items
		void								add_item		(EntityWidgetItem& item, QString group, int row);
		void								clear			();

		// Utility
		void								fit_layouts		();

	signals:
		// Signals
		void								signal_clicked	(EntityWidgetItem* item, Qt::MouseButton button);

	protected:
		// Events
		virtual void						on_clicked		(EntityWidgetItem* item, Qt::MouseButton button);

		// Accessors
		QVector<EntityWidgetItem*>			get_items		();
		QVector<const EntityWidgetItem*>	get_items		() const;

		// Creators
		virtual Result						create_item		(const QJsonValue& json, EntityWidgetItem*& item);
		virtual Result						create_data		(const QJsonValue& json, EntityWidgetItemData& data);

	private slots:
		// Item Slots
		void								slot_clicked	(Qt::MouseButton button);

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif