#ifndef INSTANCE_PROGRESS_WIDGET_H
#define INSTANCE_PROGRESS_WIDGET_H

// Qt includes
#include "UI/EntityWidget/EntityWidget.h"
#include "EditorTypeInfo.h"

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	struct SettingsDiff;
}


namespace LTTPMapTracker
{
	class ProgressItemWidget : public EntityWidget
	{
		Q_OBJECT

	public:
		// Construction & Destruction
						ProgressItemWidget		(const EditorInterface& editor_interface, QWidget* parent = nullptr);
						~ProgressItemWidget		();

		// Instance
		void			set_instance			(InstancePtr instance);
		void			clear_instance			();

	protected:
		// Events
		virtual void	on_clicked				(EntityWidgetItem* entity_item, Qt::MouseButton button) override;

		// Creators
		virtual Result	create_item				(const QJsonValue& json, EntityWidgetItem*& item);

	private slots:
		// Settings Slots
		void			slot_settings_changed	(const SettingsDiff& diff);

	private:
		// Helpers
		void			sync_from_progress		();
		void			sync_to_progress		(EntityWidgetItem* entity_item);

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif