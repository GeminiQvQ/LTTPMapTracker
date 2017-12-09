#ifndef UTILITY_MODEL_DATA_DELEGATE_H
#define UTILITY_MODEL_DATA_DELEGATE_H

// Project includes
#include "Utility/Utility.h"

// Qt includes
#include <QStyledItemDelegate>

// Stdlib includes
#include <memory>


namespace Utility
{
	class ModelDataDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

	public:
		// Construction & Destruction
					ModelDataDelegate		(QObject* parent = nullptr);
					~ModelDataDelegate		();

		// QAbstractItemDelegate Interface
		QWidget*	createEditor			(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		void		setEditorData			(QWidget* editor, const QModelIndex& index)										const override;
		void		setModelData			(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index)			const override;
		QSize		sizeHint				(const QStyleOptionViewItem& option, const QModelIndex& index)					const override;
		void		initStyleOption			(QStyleOptionViewItem* option, const QModelIndex& index)						const override;

	private:
		// Editor Slots
		void		slot_data_changed		();
		void		slot_editor_destroyed	();

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif