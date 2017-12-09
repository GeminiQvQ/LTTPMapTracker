// Project includes
#include "UI/SchemaRegionWidget/SchemaRegionPropertiesModel.h"
#include "Data/Schema/Schema.h"
#include "Utility/ModelData/ModelData.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct SchemaRegionPropertiesModel::Internal
	{
		SchemaPtr		m_schema;
		SchemaRegionPtr m_region;
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SchemaRegionPropertiesModel::SchemaRegionPropertiesModel(QObject* parent)
		: QAbstractItemModel(parent)
		, m_internal(std::make_unique<Internal>())
	{
	}

	SchemaRegionPropertiesModel::~SchemaRegionPropertiesModel()
	{
	}



	//================================================================================
	// Data
	//================================================================================

	void SchemaRegionPropertiesModel::set_region(SchemaPtr schema, SchemaRegionPtr region)
	{
		clear();

		beginResetModel();

		m_internal->m_schema = schema;
		m_internal->m_region = region;

		connect(&schema->regions(), &SchemaRegions::signal_to_be_removed, this, &SchemaRegionPropertiesModel::slot_region_to_be_removed);
		connect(&schema->regions(), &SchemaRegions::signal_modified, this, &SchemaRegionPropertiesModel::slot_region_modified);

		endResetModel();
	}

	void SchemaRegionPropertiesModel::clear()
	{
		if (m_internal->m_schema != nullptr)
		{
			m_internal->m_schema->disconnect(this);
		}

		beginResetModel();

		m_internal->m_schema = nullptr;
		m_internal->m_region = nullptr;

		endResetModel();
	}



	//================================================================================
	// QAbstractItemModel Interface
	//================================================================================

	QModelIndex SchemaRegionPropertiesModel::index(int row, int column, const QModelIndex& /*parent*/) const
	{
		return createIndex(row, column, row);
	}

	QModelIndex SchemaRegionPropertiesModel::parent(const QModelIndex& /*child*/) const
	{
		return QModelIndex();
	}

	int SchemaRegionPropertiesModel::rowCount(const QModelIndex& parent) const
	{
		return (m_internal->m_region != nullptr && !parent.isValid() ? 3 : 0);
	}

	int SchemaRegionPropertiesModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 2;
	}

	Qt::ItemFlags SchemaRegionPropertiesModel::flags(const QModelIndex& index) const
	{
		int flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

		if (index.column() == 1)
		{
			flags |= Qt::ItemIsEditable;
		}
		
		return flags;
	}

	QVariant SchemaRegionPropertiesModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
	{
		if (role == Qt::DisplayRole)
		{
			switch (section)
			{
			case 0: return "Property";
			case 1: return "Value";
			}
		}
		
		return QVariant();
	}

	QVariant SchemaRegionPropertiesModel::data(const QModelIndex& index, int role) const
	{
		if (index.column() == 0 && role == Qt::DisplayRole)
		{
			switch (index.row())
			{
			case 0: return "Name";
			case 1: return "Color";
			case 2: return "Rule";
			}
		}

		if (index.column() == 1)
		{
			if (index.row() == 0)
			{
				if (role == ModelDataRole)
				{
					ModelData data;
					data << ModelDataEntry(m_internal->m_region->get().m_name);

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 1)
			{
				if (role == Qt::DisplayRole)
				{
					auto color = m_internal->m_region->get().m_color;
					return QString("(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
				}

				if (role == ModelDataRole)
				{
					ModelData data;
					data << ModelDataEntry(m_internal->m_region->get().m_color);

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 2)
			{
				if (role == ModelDataRole)
				{
					QStringList rules;
					rules << QString();
					for (auto rule : m_internal->m_schema->rules().get())
					{
						rules << rule->get().m_name;
					}

					auto rule = m_internal->m_region->get().m_rule;

					ModelData data;
					ModelDataEntry entry;
					entry.set_value(rule ? rule->get().m_name : QString());
					entry.set_attribute(ModelDataAttribute::Enum, rules);
					entry.set_attribute(ModelDataAttribute::EnumSorted);
					data << entry;

					return QVariant::fromValue(data);
				}
			}
		}

		return QVariant();
	}

	bool SchemaRegionPropertiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
	{
		if (index.row() == 0 && role == ModelDataRole)
		{
			auto name = value.value<ModelData>()[0].get_value().toString();
			if (m_internal->m_schema->regions().find(name) != nullptr)
			{
				name = m_internal->m_region->get().m_name;
			}

			auto data = m_internal->m_region->get();
			data.m_name = name;
			m_internal->m_region->set(data);

			return true;
		}

		if (index.row() == 1 && role == ModelDataRole)
		{
			auto data = m_internal->m_region->get();
			data.m_color = value.value<ModelData>()[0].get_value().value<QColor>();
			m_internal->m_region->set(data);

			return true;
		}

		if (index.row() == 2 && role == ModelDataRole)
		{
			auto data = m_internal->m_region->get();
			data.m_rule = m_internal->m_schema->rules().find(value.value<ModelData>()[0].get_value().toString());
			m_internal->m_region->set(data);

			return true;
		}

		return false;
	}



	//================================================================================
	// Schema Slots
	//================================================================================

	void SchemaRegionPropertiesModel::slot_region_to_be_removed(int index)
	{
		if (m_internal->m_schema == nullptr)
		{
			return;
		}

		auto region = m_internal->m_schema->regions()[index];
		if (region == m_internal->m_region)
		{
			clear();
		}
	}

	void SchemaRegionPropertiesModel::slot_region_modified(int index)
	{
		if (m_internal->m_schema == nullptr)
		{
			return;
		}

		auto region = m_internal->m_schema->regions()[index];
		if (region == m_internal->m_region)
		{
			emit dataChanged(this->index(0, 1), this->index(rowCount() - 1, 1));
		}
	}
}