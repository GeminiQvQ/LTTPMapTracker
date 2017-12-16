// Project includes
#include "UI/SchemaItemWidget/SchemaItemListModel.h"
#include "Data/Schema/Schema.h"
#include "Utility/EnumReflection.h"

// Qt includes
#include <QCollator>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct SchemaItemListModel::Internal
	{
		SchemaPtr m_schema;
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SchemaItemListModel::SchemaItemListModel(QObject* parent)
		: QAbstractItemModel(parent)
		, m_internal(std::make_unique<Internal>())
	{
	}

	SchemaItemListModel::~SchemaItemListModel()
	{
	}



	//================================================================================
	// Data
	//================================================================================

	void SchemaItemListModel::set_schema(SchemaPtr schema)
	{
		clear();

		beginResetModel();

		m_internal->m_schema = schema;

		connect(&schema->items(), &SchemaItems::signal_to_be_added, this, &SchemaItemListModel::slot_item_to_be_added);
		connect(&schema->items(), &SchemaItems::signal_added, this, &SchemaItemListModel::slot_item_added);
		connect(&schema->items(), &SchemaItems::signal_to_be_removed, this, &SchemaItemListModel::slot_item_to_be_removed);
		connect(&schema->items(), &SchemaItems::signal_removed, this, &SchemaItemListModel::slot_item_removed);
		connect(&schema->items(), &SchemaItems::signal_modified, this, &SchemaItemListModel::slot_item_modified);

		endResetModel();
	}

	void SchemaItemListModel::clear()
	{
		if (m_internal->m_schema != nullptr)
		{
			m_internal->m_schema->disconnect(this);
		}

		beginResetModel();

		m_internal->m_schema = nullptr;

		endResetModel();
	}



	//================================================================================
	// QAbstractItemModel Interface
	//================================================================================

	QModelIndex SchemaItemListModel::index(int row, int column, const QModelIndex& /*parent*/) const
	{
		return createIndex(row, column, row);
	}

	QModelIndex SchemaItemListModel::parent(const QModelIndex& /*child*/) const
	{
		return QModelIndex();
	}

	int SchemaItemListModel::rowCount(const QModelIndex& parent) const
	{
		return (m_internal->m_schema != nullptr && !parent.isValid() ? m_internal->m_schema->items().get().size() : 0);
	}

	int SchemaItemListModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 2;
	}

	Qt::ItemFlags SchemaItemListModel::flags(const QModelIndex& /*index*/) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant SchemaItemListModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
	{
		if (role == Qt::DisplayRole)
		{
			switch (section)
			{
			case 0: return "Name";
			case 1: return "Map";
			}
		}

		return QVariant();
	}

	QVariant SchemaItemListModel::data(const QModelIndex& index, int role) const
	{
		auto item = m_internal->m_schema->items()[index.row()];

		if (index.column() == 0)
		{
			if (role == Qt::DisplayRole)
			{
				return item->get().m_name;
			}
		}

		if (index.column() == 1)
		{
			if (role == Qt::DisplayRole)
			{
				return EnumReflection<SchemaItemMap>::info(item->get().m_map).m_type_name;
			}
		}

		return QVariant();
	}



	//================================================================================
	// Schema Slots
	//================================================================================

	void SchemaItemListModel::slot_item_to_be_added(int index)
	{
		beginInsertRows(QModelIndex(), index, index);
	}

	void SchemaItemListModel::slot_item_added(int /*index*/)
	{
		endInsertRows();
	}

	void SchemaItemListModel::slot_item_to_be_removed(int index)
	{
		beginRemoveRows(QModelIndex(), index, index);
	}

	void SchemaItemListModel::slot_item_removed(int /*index*/)
	{
		endRemoveRows();
	}

	void SchemaItemListModel::slot_item_modified(int index)
	{
		emit dataChanged(this->index(index, 0), this->index(index, columnCount() - 1), { Qt::DisplayRole });
	}
}


namespace LTTPMapTracker
{
	//================================================================================
	// Proxy Model
	//================================================================================

	bool SchemaItemListProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
	{
		QCollator collator;
		collator.setNumericMode(true);

		auto name_a = source_left.data().toString();
		auto name_b = source_right.data().toString();
		auto map_a = source_left.sibling(source_left.row(), 1).data().toString();
		auto map_b = source_right.sibling(source_right.row(), 1).data().toString();
		
		if (collator(map_a, map_b)) return true;
		if (collator(map_b, map_a)) return false;

		return collator(name_a, name_b);
	}
}