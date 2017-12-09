// Project includes
#include "Utility/Widget/ColorButtonWidget.h"

// Qt includes
#include <QColorDialog>


namespace Utility
{
	//================================================================================
	// Construction & Destruction
	//================================================================================

	ColorButtonWidget::ColorButtonWidget(QWidget* parent)
		: QPushButton(parent)
	{
		set_color(QColor(255, 0, 0, 255));
		connect(this, &QPushButton::clicked, this, &ColorButtonWidget::open_color_dialog);
	}



	//================================================================================
	// Data
	//================================================================================

	void ColorButtonWidget::set_color(QColor color)
	{
		QString css =
		"Utility--ColorButtonWidget"
		"{"
			"margin:			1px;"
			"border-color:		#0c457e;"
			"border-style:		outset;"
			"border-radius:		3px;"
			"border-width:		1px;"
			"background-color:	%1;"
		"}";

		css = css.arg(color.name(QColor::HexArgb));

		setStyleSheet(css);

		m_color = color;

		emit signal_color_changed(color);
	}

	QColor ColorButtonWidget::get_color() const
	{
		return m_color;
	}



	//================================================================================
	// Utility
	//================================================================================

	void ColorButtonWidget::open_color_dialog()
	{
		QColorDialog d(m_color, this);
		d.setOption(QColorDialog::ShowAlphaChannel, true);

		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		set_color(d.currentColor());
	}
}