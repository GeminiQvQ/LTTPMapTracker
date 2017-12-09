#ifndef SCHEMA_RULE_PROPERTIES_MODEL_H
#define SCHEMA_RULE_PROPERTIES_MODEL_H

// Project includes
#include "EditorTypeInfo.h"

// Qt includes
#include <QAbstractItemModel>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class SchemaRulePropertiesModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		// Construction & Destruction
								SchemaRulePropertiesModel	(EditorInterface& editor_interface, QObject* parent = nullptr);
								~SchemaRulePropertiesModel	();

		// Data
		void					set_rule					(SchemaPtr schema, SchemaRulePtr rule);
		void					clear						();

		void					insert_entry				(int index = -1);
		void					remove_entries				(QVector<int> indices);

		// QAbstractItemModel Interface
		virtual QModelIndex		index						(int row, int column, const QModelIndex& parent = QModelIndex())			const	override;
		virtual QModelIndex		parent						(const QModelIndex& child)													const	override;
		virtual int				rowCount					(const QModelIndex& parent = QModelIndex())									const	override;
		virtual int				columnCount					(const QModelIndex& parent = QModelIndex())									const	override;
		virtual Qt::ItemFlags	flags						(const QModelIndex& index)													const	override;
		virtual QVariant		headerData					(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)		const	override;
		virtual QVariant		data						(const QModelIndex& index, int role = Qt::DisplayRole)						const	override;
		virtual bool			setData						(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)			override;

	private slots:
		// Schema Slots
		void					slot_rule_to_be_removed		(int index);
		void					slot_rule_modified			(int index);

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif