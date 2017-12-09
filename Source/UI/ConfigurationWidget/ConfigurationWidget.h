#ifndef CONFIGURATION_WIDGET_H
#define CONFIGURATION_WIDGET_H

// Project includes
#include "Data/Configuration.h"

// Qt includes
#include <QWidget>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class ConfigurationWidget : public QWidget
	{
	public:
		// Construction & Destruction
				ConfigurationWidget		(EditorInterface& editor_interface, QWidget* parent = nullptr);
				~ConfigurationWidget	();

		// Configuration
		void	set_configuration		(ConfigurationPtr configuration);
		void	clear_configuration		();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif