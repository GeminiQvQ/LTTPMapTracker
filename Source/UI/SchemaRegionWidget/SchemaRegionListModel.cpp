// Project includes
#include "UI/SchemaRegionWidget/SchemaRegionListModel.h"
#include "Data/Schema/Schema.h"
#include "Data/Settings.h"
#include "EditorInterface.h"

// Qt includes
#include <QCollator>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct SchemaRegionListModel::Internal
	{
		const EditorInterface&	m_editor_interface;
		SchemaPtr				m_schema;

		Internal(const EditorInterface& editor_interface)
			: m_editor_interface(editor_interface)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SchemaRegionListModel::SchemaRegionListModel(const EditorInterface& editor_interface, QObject* parent)
		: QAbstractItemModel(parent)
		, m_internal(std::make_unique<Internal>(editor_interface))
	{
	}

	SchemaRegionListModel::~SchemaRegionListModel()
	{
	}



	//================================================================================
	// Data
	//================================================================================

	void SchemaRegionListModel::set_schema(SchemaPtr schema)
	{
		clear();

		beginResetModel();
		
		m_internal->m_schema = schema;

		connect(&schema->regions(), &SchemaRegions::signal_to_be_added, this, &SchemaRegionListModel::slot_region_to_be_added);
		connect(&schema->regions(), &SchemaRegions::signal_added, this, &SchemaRegionListModel::slot_region_added);
		connect(&schema->regions(), &SchemaRegions::signal_to_be_removed, this, &SchemaRegionListModel::slot_region_to_be_removed);
		connect(&schema->regions(), &SchemaRegions::signal_removed, this, &SchemaRegionListModel::slot_region_removed);
		connect(&schema->regions(), &SchemaRegions::signal_modified, this, &SchemaRegionListModel::slot_region_modified);

		endResetModel();
	}

	void SchemaRegionListModel::clear()
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

	QModelIndex SchemaRegionListModel::index(int row, int column, const QModelIndex& /*parent*/) const
	{
		return createIndex(row, column, row);
	}

	QModelIndex SchemaRegionListModel::parent(const QModelIndex& /*child*/) const
	{
		return QModelIndex();
	}

	int SchemaRegionListModel::rowCount(const QModelIndex& parent) const
	{
		return (m_internal->m_schema != nullptr && !parent.isValid() ? m_internal->m_schema->regions().get().size() : 0);
	}

	int SchemaRegionListModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 1;
	}

	Qt::ItemFlags SchemaRegionListModel::flags(const QModelIndex& /*index*/) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant SchemaRegionListModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
	{
		if (role == Qt::DisplayRole)
		{
			switch (section)
			{
			case 0: return "Name";
			}
		}

		return QVariant();
	}

	QVariant SchemaRegionListModel::data(const QModelIndex& index, int role) const
	{
		auto region = m_internal->m_schema->regions()[index.row()];

		if (index.column() == 0)
		{
			if (role == Qt::DisplayRole)
			{
				return region->get().m_name;
			}

			if (role == Qt::DecorationRole)
			{
				return region->get().m_color;
			}

			if (role == Qt::ForegroundRole && m_internal->m_editor_interface.get_settings().get().m_editor_show_unused_regions)
			{
				auto items = m_internal->m_schema->items().get();
				bool items_found = std::any_of(items.begin(), items.end(), [region] (SchemaItemCPtr item)
				{
					return (item->get().m_region == region);
				});

				return (!items_found ? QColor(128, 128, 128) : QVariant());
			}
		}

		return QVariant();
	}



	//================================================================================
	// Schema Slots
	//================================================================================

	void SchemaRegionListModel::slot_region_to_be_added(int index)
	{
		beginInsertRows(QModelIndex(), index, index);
	}

	void SchemaRegionListModel::slot_region_added(int /*index*/)
	{
		endInsertRows();
	}

	void SchemaRegionListModel::slot_region_to_be_removed(int index)
	{
		beginRemoveRows(QModelIndex(), index, index);
	}

	void SchemaRegionListModel::slot_region_removed(int /*index*/)
	{
		endRemoveRows();
	}

	void SchemaRegionListModel::slot_region_modified(int index)
	{
		emit dataChanged(this->index(index, 0), this->index(index, columnCount() - 1), { Qt::DisplayRole });
	}
}


namespace LTTPMapTracker
{
	//================================================================================
	// Proxy Model
	//================================================================================

	bool SchemaRegionListProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
	{
		QCollator collator;
		collator.setNumericMode(true);

		return collator(source_left.data().toString(), source_right.data().toString());
	}
}