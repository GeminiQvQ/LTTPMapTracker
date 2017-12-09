#ifndef SCHEMA_ITEM_LIST_MODEL_H
#define SCHEMA_ITEM_LIST_MODEL_H

// Project includes
#include "EditorTypeInfo.h"

// Qt includes
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	// Model
	//--------------------------------------------------------------------------------

	class SchemaItemListModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		// Construction & Destruction
								SchemaItemListModel		(QObject* parent = nullptr);
								~SchemaItemListModel	();

		// Data
		void					set_schema				(SchemaPtr schema);
		void					clear					();

		// QAbstractItemModel Interface
		virtual QModelIndex		index					(int row, int column, const QModelIndex& parent = QModelIndex())			const	override;
		virtual QModelIndex		parent					(const QModelIndex& child)													const	override;
		virtual int				rowCount				(const QModelIndex& parent = QModelIndex())									const	override;
		virtual int				columnCount				(const QModelIndex& parent = QModelIndex())									const	override;
		virtual Qt::ItemFlags	flags					(const QModelIndex& index)													const	override;
		virtual QVariant		headerData				(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)		const	override;
		virtual QVariant		data					(const QModelIndex& index, int role = Qt::DisplayRole)						const	override;

	private slots:
		// Schema Slots
		void					slot_item_to_be_added	(int index);
		void					slot_item_added			(int index);
		void					slot_item_to_be_removed	(int index);
		void					slot_item_removed		(int index);
		void					slot_item_modified		(int index);

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};


	// Proxy Model
	//--------------------------------------------------------------------------------

	class SchemaItemListProxyModel : public QSortFilterProxyModel
	{
	protected:
		virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
	};
}

#endif