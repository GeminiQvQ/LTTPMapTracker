// Project includes
#include "UI/SchemaItemWidget/SchemaItemPropertiesModel.h"
#include "Data/Schema/Schema.h"
#include "Utility/ModelData/ModelData.h"
#include "Utility/EnumReflection.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct SchemaItemPropertiesModel::Internal
	{
		SchemaPtr		m_schema;
		SchemaItemPtr	m_item;
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SchemaItemPropertiesModel::SchemaItemPropertiesModel(QObject* parent)
		: QAbstractItemModel(parent)
		, m_internal(std::make_unique<Internal>())
	{
	}

	SchemaItemPropertiesModel::~SchemaItemPropertiesModel()
	{
	}



	//================================================================================
	// Data
	//================================================================================

	void SchemaItemPropertiesModel::set_item(SchemaPtr schema, SchemaItemPtr item)
	{
		clear();

		beginResetModel();

		m_internal->m_schema = schema;
		m_internal->m_item = item;

		connect(&schema->items(), &SchemaItems::signal_to_be_removed, this, &SchemaItemPropertiesModel::slot_item_to_be_removed);
		connect(&schema->items(), &SchemaItems::signal_modified, this, &SchemaItemPropertiesModel::slot_item_modified);

		endResetModel();
	}

	void SchemaItemPropertiesModel::clear()
	{
		if (m_internal->m_schema != nullptr)
		{
			m_internal->m_schema->disconnect(this);
		}

		beginResetModel();

		m_internal->m_schema = nullptr;
		m_internal->m_item = nullptr;

		endResetModel();
	}



	//================================================================================
	// QAbstractItemModel Interface
	//================================================================================

	QModelIndex SchemaItemPropertiesModel::index(int row, int column, const QModelIndex& /*parent*/) const
	{
		return createIndex(row, column, row);
	}

	QModelIndex SchemaItemPropertiesModel::parent(const QModelIndex& /*child*/) const
	{
		return QModelIndex();
	}

	int SchemaItemPropertiesModel::rowCount(const QModelIndex& parent) const
	{
		return (m_internal->m_item != nullptr && !parent.isValid() ? 6 : 0);
	}

	int SchemaItemPropertiesModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 2;
	}

	Qt::ItemFlags SchemaItemPropertiesModel::flags(const QModelIndex& index) const
	{
		int flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

		if (index.column() == 1)
		{
			flags |= Qt::ItemIsEditable;
		}

		return flags;
	}

	QVariant SchemaItemPropertiesModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
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

	QVariant SchemaItemPropertiesModel::data(const QModelIndex& index, int role) const
	{
		if (index.column() == 0 && role == Qt::DisplayRole)
		{
			switch (index.row())
			{
			case 0: return "Name";
			case 1: return "Map";
			case 2: return "Position";
			case 3: return "Region";
			case 4: return "Rule";
			case 5: return "Rule Access";
			}
		}

		if (index.column() == 1)
		{
			if (index.row() == 0)
			{
				if (role == ModelDataRole)
				{
					ModelData data;
					data << ModelDataEntry(m_internal->m_item->get().m_name);

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 1)
			{
				if (role == Qt::DisplayRole)
				{
					return EnumReflection<SchemaItemMap>::info(m_internal->m_item->get().m_map).m_display_name;
				}

				if (role == ModelDataRole)
				{
					QStringList maps;
					for (int i = 0; i < EnumReflection<SchemaItemMap>::num(); ++i)
					{
						maps << EnumReflection<SchemaItemMap>::info(i).m_display_name;
					}

					ModelData data;
					ModelDataEntry entry;
					entry.set_value(EnumReflection<SchemaItemMap>::index(m_internal->m_item->get().m_map));
					entry.set_attribute(ModelDataAttribute::Enum, maps);
					data << entry;

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 2)
			{
				if (role == Qt::DisplayRole)
				{
					return QString("(%1, %2)").arg(m_internal->m_item->get().m_position.x(), 0, 'f', 1).arg(m_internal->m_item->get().m_position.y(), 0, 'f', 1);
				}

				if (role == ModelDataRole)
				{
					ModelData data;
					data << ModelDataEntry(m_internal->m_item->get().m_position.x());
					data << ModelDataEntry(m_internal->m_item->get().m_position.y());

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 3)
			{
				if (role == ModelDataRole)
				{
					QStringList regions;
					regions << QString();
					for (auto region : m_internal->m_schema->regions().get())
					{
						regions << region->get().m_name;
					}

					auto region = m_internal->m_item->get().m_region;

					ModelData data;
					ModelDataEntry entry;
					entry.set_value(region ? region->get().m_name : QString());
					entry.set_attribute(ModelDataAttribute::Enum, regions);
					entry.set_attribute(ModelDataAttribute::EnumSorted);
					data << entry;

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 4)
			{
				if (role == ModelDataRole)
				{
					QStringList rules;
					rules << QString();
					for (auto rule : m_internal->m_schema->rules().get())
					{
						rules << rule->get().m_name;
					}

					auto rule = m_internal->m_item->get().m_rule;

					ModelData data;
					ModelDataEntry entry;
					entry.set_value(rule ? rule->get().m_name : QString());
					entry.set_attribute(ModelDataAttribute::Enum, rules);
					entry.set_attribute(ModelDataAttribute::EnumSorted);
					data << entry;

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 5)
			{
				if (role == Qt::DisplayRole)
				{
					return EnumReflection<SchemaRuleAccessType>::info(m_internal->m_item->get().m_rule_access).m_display_name;
				}

				if (role == ModelDataRole)
				{
					QStringList rule_access_names;
					for (int i = 0; i < EnumReflection<SchemaRuleAccessType>::num(); ++i)
					{
						rule_access_names << EnumReflection<SchemaRuleAccessType>::info(i).m_display_name;
					}

					ModelData data;
					ModelDataEntry entry;
					entry.set_value(EnumReflection<SchemaRuleAccessType>::index(m_internal->m_item->get().m_rule_access));
					entry.set_attribute(ModelDataAttribute::Enum, rule_access_names);
					data << entry;

					return QVariant::fromValue(data);
				}
			}
		}

		return QVariant();
	}

	bool SchemaItemPropertiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
	{
		if (index.row() == 0 && role == ModelDataRole)
		{
			auto name = value.value<ModelData>()[0].get_value().toString();
			if (m_internal->m_schema->items().find(name) != nullptr)
			{
				name = m_internal->m_item->get().m_name;
			}

			auto data = m_internal->m_item->get();
			data.m_name = name;
			m_internal->m_item->set(data);

			return true;
		}

		if (index.row() == 1 && role == ModelDataRole)
		{
			auto data = m_internal->m_item->get();
			data.m_map = EnumReflection<SchemaItemMap>::info(value.value<ModelData>()[0].get_value().toInt()).m_type;
			m_internal->m_item->set(data);

			return true;
		}

		if (index.row() == 2 && role == ModelDataRole)
		{
			auto data = m_internal->m_item->get();
			data.m_position.setX(value.value<ModelData>()[0].get_value().toFloat());
			data.m_position.setY(value.value<ModelData>()[1].get_value().toFloat());
			m_internal->m_item->set(data);

			return true;
		}

		if (index.row() == 3 && role == ModelDataRole)
		{
			auto data = m_internal->m_item->get();
			data.m_region = m_internal->m_schema->regions().find(value.value<ModelData>()[0].get_value().toString());
			m_internal->m_item->set(data);

			return true;
		}

		if (index.row() == 4 && role == ModelDataRole)
		{
			auto data = m_internal->m_item->get();
			data.m_rule = m_internal->m_schema->rules().find(value.value<ModelData>()[0].get_value().toString());
			m_internal->m_item->set(data);

			return true;
		}

		if (index.row() == 5 && role == ModelDataRole)
		{
			auto data = m_internal->m_item->get();
			data.m_rule_access = EnumReflection<SchemaRuleAccessType>::info(value.value<ModelData>()[0].get_value().toInt()).m_type;
			m_internal->m_item->set(data);

			return true;
		}

		return false;
	}



	//================================================================================
	// Schema Slots
	//================================================================================

	void SchemaItemPropertiesModel::slot_item_to_be_removed(int index)
	{
		if (m_internal->m_schema == nullptr)
		{
			return;
		}

		auto item = m_internal->m_schema->items()[index];
		if (item == m_internal->m_item)
		{
			clear();
		}
	}

	void SchemaItemPropertiesModel::slot_item_modified(int index)
	{
		if (m_internal->m_schema == nullptr)
		{
			return;
		}

		auto item = m_internal->m_schema->items()[index];
		if (item == m_internal->m_item)
		{
			emit dataChanged(this->index(0, 1), this->index(rowCount() - 1, 1));
		}
	}
}