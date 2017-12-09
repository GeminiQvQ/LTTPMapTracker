#ifndef MAP_ENTITY_WIDGET_H
#define MAP_ENTITY_WIDGET_H

// Qt includes
#include "EditorTypeInfo.h"
#include "UI/EntityWidget/EntityWidget.h"
#include "UI/EntityWidget/EntityWidgetItem.h"


namespace LTTPMapTracker
{
	class MapEntityWidget : public EntityWidget
	{
	public:
		// Construction & Destruction
						MapEntityWidget		(const EditorInterface& editor_interface, Qt::Alignment alignment, QWidget* parent = nullptr);
						~MapEntityWidget	();

		// Item
		void			set_item			(InstanceItemPtr instance_item);

	protected:
		// Events
		virtual void	on_clicked			(EntityWidgetItem* entity_item, Qt::MouseButton button) override;

	private:
		// Helpers
		void			sync				();

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif