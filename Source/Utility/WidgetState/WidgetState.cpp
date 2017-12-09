// Project includes
#include "Utility/WidgetState/WidgetState.h"

// Qt includes
#include <QDataStream>
#include <QHeaderView>
#include <QSplitter>
#include <QTreeView>


namespace Utility
{
	//================================================================================
	// QWidget
	//================================================================================

	template <> void save_widget_state(const QWidget& widget, QDataStream& data)
	{
		QString name = widget.metaObject()->className();

		if (name == "QHeaderView") save_widget_state(static_cast<const QHeaderView&>(widget), data);
		if (name == "QTreeView") save_widget_state(static_cast<const QTreeView&>(widget), data);
		if (name == "QSplitter") save_widget_state(static_cast<const QSplitter&>(widget), data);
	}

	template <> void load_widget_state(QWidget& widget, QDataStream& data)
	{
		QString name = widget.metaObject()->className();

		if (name == "QHeaderView") load_widget_state(static_cast<QHeaderView&>(widget), data);
		if (name == "QTreeView") load_widget_state(static_cast<QTreeView&>(widget), data);
		if (name == "QSplitter") load_widget_state(static_cast<QSplitter&>(widget), data);
	}



	//================================================================================
	// QHeaderView
	//================================================================================

	template <> void save_widget_state(const QHeaderView& widget, QDataStream& data)
	{
		static const int s_version = 1;
		data << s_version;

		data << widget.count();
		for (int i = 0; i < widget.count() - 1; ++i)
		{
			data << widget.sectionSize(i);
		}
	}

	template <> void load_widget_state(QHeaderView& widget, QDataStream& data)
	{
		int version = 0;
		data >> version;

		int num = 0;
		data >> num;

		for (int i = 0; i < widget.count() - 1 && i < num; ++i)
		{
			int size = widget.sectionSize(i);
			data >> size;
			widget.resizeSection(i, size);
		}
	}



	//================================================================================
	// QTreeView
	//================================================================================

	template <> void save_widget_state(const QTreeView& widget, QDataStream& data)
	{
		static const int s_version = 1;
		data << s_version;

		save_widget_state(*widget.header(), data);
	}

	template <> void load_widget_state(QTreeView& widget, QDataStream& data)
	{
		int version = 0;
		data >> version;

		load_widget_state(*widget.header(), data);
	}



	//================================================================================
	// QSplitter
	//================================================================================

	template <> void save_widget_state(const QSplitter& widget, QDataStream& data)
	{
		static const int s_version = 1;
		data << s_version;

		data << widget.sizes();
	}

	template <> void load_widget_state(QSplitter& widget, QDataStream& data)
	{
		int version = 0;
		data >> version;
		
		QList<int> sizes;
		data >> sizes;

		widget.setSizes(sizes);
	}
}