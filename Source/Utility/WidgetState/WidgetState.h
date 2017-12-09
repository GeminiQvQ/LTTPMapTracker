#ifndef UTILITY_WIDGET_STATE_H
#define UTILITY_WIDGET_STATE_H

// Project includes
#include "Utility/Utility.h"

// Forward declarations
class QDataStream;
class QHeaderView;
class QSplitter;
class QTreeView;
class QWidget;


namespace Utility
{
	// Interface
	//--------------------------------------------------------------------------------

	template <typename T>
	void save_widget_state(const T& widget, QDataStream& data);

	template <typename T>
	void load_widget_state(T& widget, QDataStream& data);


	// Specialization
	//--------------------------------------------------------------------------------

	template <> void save_widget_state(const QWidget& widget, QDataStream& data);
	template <> void load_widget_state(QWidget& widget, QDataStream& data);

	template <> void save_widget_state(const QHeaderView& widget, QDataStream& data);
	template <> void load_widget_state(QHeaderView& widget, QDataStream& data);

	template <> void save_widget_state(const QTreeView& widget, QDataStream& data);
	template <> void load_widget_state(QTreeView& widget, QDataStream& data);

	template <> void save_widget_state(const QSplitter& widget, QDataStream& data);
	template <> void load_widget_state(QSplitter& widget, QDataStream& data);
}

#endif