#ifndef UTILITY_WIDGET_STATE_MANAGER_H
#define UTILITY_WIDGET_STATE_MANAGER_H

// Project includes
#include "Utility/Utility.h"

// Qt includes
#include <QWidget>

// Stdlib includes
#include <memory>

// Forward declarations
class QSettings;


namespace Utility
{
	// Types
	//--------------------------------------------------------------------------------

	class WidgetStateDelegate
	{
	public:
		virtual void on_load_state(QWidget& widget) = 0;
		virtual void on_save_state(QWidget& widget) = 0;
	};

	using WidgetStateDelegatePtr = std::shared_ptr<WidgetStateDelegate>;


	// Widget State Manager
	//--------------------------------------------------------------------------------

	class WidgetStateManager : public QObject
	{
	public:
		// Construction & Destruction
						WidgetStateManager	(QObject* parent = nullptr);
						~WidgetStateManager	();

		// Registration
		void			register_widget		(QWidget& widget);

		// Settings
		void			load				(const QSettings& settings);
		void			save				(QSettings& settings);

		// Delegate
		void			add_delegate		(WidgetStateDelegatePtr state_delegate);
		void			remove_delegate		(WidgetStateDelegatePtr state_delegate);

	protected:
		// Qt Events
		virtual bool	eventFilter			(QObject* object, QEvent* event) override;

	private:
		// State
		void			load_state			(QWidget& widget);
		void			save_state			(QWidget& widget);

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif