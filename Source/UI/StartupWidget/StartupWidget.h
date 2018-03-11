#ifndef STARTUP_WIDGET_H
#define STARTUP_WIDGET_H

// Project includes
#include "EditorTypeInfo.h"

// Qt includes
#include <QDialog>


namespace LTTPMapTracker
{
	class StartupWidget : public QDialog
	{
	public:
		// Construction & Destruction
				StartupWidget				(EditorInterface& editor_interface, QWidget* parent = nullptr);
				~StartupWidget				();

		// Accessors
		QString get_selected_configuration	() const;

	private slots:
		// UI Slots
		void	slot_show_changed			(bool enabled);
		void	slot_autorun_changed		(bool enabled);

	private:
		// Helpers
		void	validate_ui					();

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif