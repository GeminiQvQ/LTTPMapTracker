#ifndef UTILITY_WINDOW_MANAGER_H
#define UTILITY_WINDOW_MANAGER_H

// Project includes
#include "Utility/Utility.h"

// Qt includes
#include <QVector>

// Stdlib includes
#include <functional>
#include <memory>

// Forward declarations
class QMainWindow;
class QMenu;
class QSettings;
class QWidget;


namespace Utility
{
	// Types
	//--------------------------------------------------------------------------------

	using SettingsCreator = std::function<std::unique_ptr<QSettings>()>;


	// Window Manager
	//--------------------------------------------------------------------------------

	class WindowManager
	{
	public:
		// Construction & Destruction
				WindowManager			(QMainWindow& main_window, QMenu& menu, SettingsCreator settings_creator);
				~WindowManager			();

		// Configuration Management
		int		add_configuration		(QString name);
		void	enable_configuration	(int configuration);

		// Widget Management
		void	add_dockable_widget		(QWidget& widget, Qt::DockWidgetArea dock_area, QVector<int> configurations = {0});

		// State
		void	save					();
		void	load					();
		void	reset					();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif