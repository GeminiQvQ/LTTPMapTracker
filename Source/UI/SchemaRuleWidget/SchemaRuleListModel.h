#ifndef SCHEMA_RULE_LIST_MODEL_H
#define SCHEMA_RULE_LIST_MODEL_H

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

	class SchemaRuleListModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		// Construction & Destruction
								SchemaRuleListModel		(const EditorInterface& editor_interface, QObject* parent = nullptr);
								~SchemaRuleListModel	();

		// Data
		void					set_schema				(SchemaPtr schema);
		void					clear					();

		// QAbstractItemModel Interface
		virtual QModelIndex		index					(int row, int column, const QModelIndex& parent = QModelIndex())		const	override;
		virtual QModelIndex		parent					(const QModelIndex& child)												const	override;
		virtual int				rowCount				(const QModelIndex& parent = QModelIndex())								const	override;
		virtual int				columnCount				(const QModelIndex& parent = QModelIndex())								const	override;
		virtual Qt::ItemFlags	flags					(const QModelIndex& index)												const	override;
		virtual QVariant		headerData				(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)	const	override;
		virtual QVariant		data					(const QModelIndex& index, int role = Qt::DisplayRole)					const	override;
		virtual bool			setData					(const QModelIndex& index, const QVariant& value, int role)						override;

	private slots:
		// Schema Slots
		void					slot_rule_to_be_added	(int index);
		void					slot_rule_added			(int index);
		void					slot_rule_to_be_removed	(int index);
		void					slot_rule_removed		(int index);
		void					slot_rule_modified		(int index);

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};


	// Proxy Model
	//--------------------------------------------------------------------------------

	class SchemaRuleListProxyModel : public QSortFilterProxyModel
	{
	protected:
		virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
	};
}

#endif