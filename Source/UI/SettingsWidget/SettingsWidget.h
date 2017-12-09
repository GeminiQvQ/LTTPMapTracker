#ifndef SETTINGS_WIDGET_H
#define SETTINGS_WIDGET_H

// Qt includes
#include <QDialog>

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	class Settings;
}


namespace LTTPMapTracker
{
	class SettingsWidget : public QDialog
	{
	public:
		// Construction & Destruction
				SettingsWidget	(Settings& settings, QWidget* parent = nullptr);
				~SettingsWidget	();

	private slots:
		// UI Slots
		void	slot_ok			();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif