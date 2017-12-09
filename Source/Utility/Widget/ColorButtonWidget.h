#ifndef UTILITY_COLOR_BUTTON_WIDGET_H
#define UTILITY_COLOR_BUTTON_WIDGET_H

// Project includes
#include "Utility/Utility.h"

// Qt includes
#include <QColor>
#include <QPushButton>


namespace Utility
{
	class ColorButtonWidget : public QPushButton
	{
		Q_OBJECT

	public:
		// Construction & Destruction
				ColorButtonWidget		(QWidget* parent = nullptr);
		
		// Data
		void	set_color				(QColor color);
		QColor	get_color				() const;
		
		// Utility
		void	open_color_dialog		();

	signals:
		// Signals
		void	signal_color_changed	(QColor color);

	private:
		QColor	m_color;
	};
}

#endif