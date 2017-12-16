// Project includes
#include "UI/SchemaRuleWidget/SchemaRulePropertiesModel.h"
#include "Data/Database/ItemDatabase.h"
#include "Data/Database/LocationDatabase.h"
#include "Data/Schema/Schema.h"
#include "Data/DataModel.h"
#include "Utility/ModelData/ModelData.h"
#include "Utility/EnumReflection.h"
#include "EditorInterface.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct SchemaRulePropertiesModel::Internal
	{
		EditorInterface& m_editor_interface;
		SchemaPtr		 m_schema;
		SchemaRulePtr	 m_rule;

		Internal(EditorInterface& editor_interface)
			: m_editor_interface(editor_interface)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SchemaRulePropertiesModel::SchemaRulePropertiesModel(EditorInterface& editor_interface, QObject* parent)
		: QAbstractItemModel(parent)
		, m_internal(std::make_unique<Internal>(editor_interface))
	{
	}

	SchemaRulePropertiesModel::~SchemaRulePropertiesModel()
	{
	}



	//================================================================================
	// Data
	//================================================================================

	void SchemaRulePropertiesModel::set_rule(SchemaPtr schema, SchemaRulePtr rule)
	{
		clear();

		beginResetModel();

		m_internal->m_schema = schema;
		m_internal->m_rule = rule;

		connect(&schema->rules(), &SchemaRules::signal_to_be_removed, this, &SchemaRulePropertiesModel::slot_rule_to_be_removed);
		connect(&schema->rules(), &SchemaRules::signal_modified, this, &SchemaRulePropertiesModel::slot_rule_modified);

		endResetModel();
	}

	void SchemaRulePropertiesModel::clear()
	{
		if (m_internal->m_schema != nullptr)
		{
			m_internal->m_schema->disconnect(this);
		}

		beginResetModel();

		m_internal->m_schema = nullptr;
		m_internal->m_rule = nullptr;

		endResetModel();
	}

	//--------------------------------------------------------------------------------

	void SchemaRulePropertiesModel::insert_entry(int index)
	{
		index = (index == -1 ? rowCount() : index);
		
		beginInsertRows(QModelIndex(), index, index);

		auto rule_data = m_internal->m_rule->get();
		rule_data.m_entries.insert(index, SchemaRuleEntry());
		m_internal->m_rule->set(rule_data);

		endInsertRows();
	}

	void SchemaRulePropertiesModel::remove_entries(QVector<int> indices)
	{
		qSort(indices.begin(), indices.end(), [] (int a, int b)
		{
			return (a > b);
		});
		
		for (auto index : indices)
		{
			beginRemoveRows(QModelIndex(), index, index);

			auto rule_data = m_internal->m_rule->get();
			rule_data.m_entries.remove(index);
			m_internal->m_rule->set(rule_data);

			endRemoveRows();
		}
	}



	//================================================================================
	// QAbstractItemModel Interface
	//================================================================================

	QModelIndex SchemaRulePropertiesModel::index(int row, int column, const QModelIndex& /*parent*/) const
	{
		return createIndex(row, column, row);
	}

	QModelIndex SchemaRulePropertiesModel::parent(const QModelIndex& /*child*/) const
	{
		return QModelIndex();
	}

	int SchemaRulePropertiesModel::rowCount(const QModelIndex& parent) const
	{
		return (m_internal->m_rule != nullptr && !parent.isValid() ? m_internal->m_rule->get().m_entries.size() : 0);
	}

	int SchemaRulePropertiesModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 5;
	}

	Qt::ItemFlags SchemaRulePropertiesModel::flags(const QModelIndex& index) const
	{
		int flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

		if (index.column() == 4 && index.row() == rowCount() - 1)
		{
			flags &= ~Qt::ItemIsEditable;
		}

		return flags;
	}

	QVariant SchemaRulePropertiesModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
	{
		if (role == Qt::DisplayRole)
		{
			switch (section)
			{
			case 0: return "Open Brackets";
			case 1: return "Type";
			case 2: return "Value";
			case 3: return "Close Brackets";
			case 4: return "Operator";
			}
		}
		
		return QVariant();
	}

	QVariant SchemaRulePropertiesModel::data(const QModelIndex& index, int role) const
	{
		auto& rule_entry = m_internal->m_rule->get().m_entries[index.row()];

		// Open bracket.
		if (index.column() == 0)
		{
			if (role == Qt::DisplayRole)
			{
				return (rule_entry.m_brackets_open > 0 ? QString("( ").repeated(rule_entry.m_brackets_open) : "-");
			}

			if (role == ModelDataRole)
			{
				ModelData data;
				ModelDataEntry entry;
				entry.set_value(rule_entry.m_brackets_open);
				entry.set_attribute(ModelDataAttribute::MinValue, 0);
				data << entry;

				return QVariant::fromValue(data);
			}

			if (role == Qt::TextAlignmentRole)
			{
				return Qt::AlignCenter;
			}
		}

		// Type.
		if (index.column() == 1)
		{
			if (role == Qt::DisplayRole)
			{
				return EnumReflection<SchemaRuleType>::info(rule_entry.m_type).m_display_name;
			}

			if (role == ModelDataRole)
			{
				QStringList types;
				for (int i = 0; i < EnumReflection<SchemaRuleType>::num(); ++i)
				{
					types << EnumReflection<SchemaRuleType>::info(i).m_display_name;
				}

				ModelData data;
				ModelDataEntry entry;
				entry.set_value(EnumReflection<SchemaRuleType>::index(rule_entry.m_type));
				entry.set_attribute(ModelDataAttribute::Enum, types);
				data << entry;

				return QVariant::fromValue(data);
			}
		}

		// Value.
		if (index.column() == 2)
		{
			if (role == Qt::DisplayRole)
			{
				if (rule_entry.m_value.isEmpty())
				{
					return "<None>";
				}
			}

			if (role == ModelDataRole)
			{
				auto& item_db = m_internal->m_editor_interface.get_data_model().get_item_db();
				auto& location_db = m_internal->m_editor_interface.get_data_model().get_location_db();
				
				QStringList type_names;
				type_names << QString();

				QStringList display_names;
				display_names << QString();

				if (rule_entry.m_type == SchemaRuleType::ProgressItem)
				{
					for (auto item : item_db.get_items())
					{
						type_names << item->m_entity->m_type_name;
						display_names << item->m_entity->m_display_name;
					}
				}

				if (rule_entry.m_type == SchemaRuleType::ProgressLocation)
				{
					for (auto location : location_db.get_locations())
					{
						type_names << location->m_entity->m_type_name;
						display_names << location->m_entity->m_display_name;
					}
				}

				if (rule_entry.m_type == SchemaRuleType::ProgressSpecial)
				{
					for (int i = 0; i < EnumReflection<SchemaRuleTypeProgressSpecial>::num(); ++i)
					{
						type_names << EnumReflection<SchemaRuleTypeProgressSpecial>::info(i).m_type_name;
						display_names << EnumReflection<SchemaRuleTypeProgressSpecial>::info(i).m_display_name;
					}
				}

				if (rule_entry.m_type == SchemaRuleType::SchemaRule)
				{
					for (auto rule : m_internal->m_schema->rules().get())
					{
						type_names << rule->get().m_name;
						display_names << rule->get().m_name;
					}
				}

				if (rule_entry.m_type == SchemaRuleType::SchemaItem)
				{
					for (auto item : m_internal->m_schema->items().get())
					{
						type_names << item->get().m_name;
						display_names << item->get().m_name;
					}
				}

				if (rule_entry.m_type == SchemaRuleType::SchemaRegion)
				{
					for (auto region : m_internal->m_schema->regions().get())
					{
						type_names << region->get().m_name;
						display_names << region->get().m_name;
					}
				}

				int type_name_index = type_names.indexOf(rule_entry.m_value);
				auto display_name = (type_name_index >= 0 ? display_names[type_name_index] : QString());

				ModelData data;
				ModelDataEntry entry;
				entry.set_value(display_name);
				entry.set_attribute(ModelDataAttribute::Enum, display_names);
				entry.set_attribute(ModelDataAttribute::EnumSorted);
				entry.set_attribute(ModelDataAttribute::Custom1, type_names);
				data << entry;

				return QVariant::fromValue(data);
			}
		}

		// Close bracket.
		if (index.column() == 3)
		{
			if (role == Qt::DisplayRole)
			{
				return (rule_entry.m_brackets_close > 0 ? QString(") ").repeated(rule_entry.m_brackets_close) : "-");
			}

			if (role == ModelDataRole)
			{
				ModelData data;
				ModelDataEntry entry;
				entry.set_value(rule_entry.m_brackets_close);
				entry.set_attribute(ModelDataAttribute::MinValue, 0);
				data << entry;

				return QVariant::fromValue(data);
			}

			if (role == Qt::TextAlignmentRole)
			{
				return Qt::AlignCenter;
			}
		}

		// Operator.
		if (index.column() == 4 && index.row() < rowCount() - 1)
		{
			if (role == Qt::DisplayRole)
			{
				return EnumReflection<SchemaRuleOperator>::info(rule_entry.m_operator).m_display_name;
			}

			if (role == ModelDataRole)
			{
				QStringList types;
				for (int i = 0; i < EnumReflection<SchemaRuleOperator>::num(); ++i)
				{
					types << EnumReflection<SchemaRuleOperator>::info(i).m_display_name;
				}

				ModelData data;
				ModelDataEntry entry;
				entry.set_value(EnumReflection<SchemaRuleOperator>::index(rule_entry.m_operator));
				entry.set_attribute(ModelDataAttribute::Enum, types);
				data << entry;

				return QVariant::fromValue(data);
			}
		}

		return QVariant();
	}

	bool SchemaRulePropertiesModel::setData(const QModelIndex& index, const QVariant& value, int /*role*/)
	{
		auto rule_entry = m_internal->m_rule->get().m_entries[index.row()];

		// Open bracket.
		if (index.column() == 0)
		{
			rule_entry.m_brackets_open = value.value<ModelData>()[0].get_value().toInt();
		}

		// Type.
		if (index.column() == 1)
		{
			rule_entry.m_type = EnumReflection<SchemaRuleType>::info(value.value<ModelData>()[0].get_value().toInt()).m_type;
			rule_entry.m_value = QString();
		}

		// Value.
		if (index.column() == 2)
		{
			auto model_data = value.value<ModelData>()[0];
			auto type_names = model_data.get_attribute(ModelDataAttribute::Custom1).toStringList();
			auto display_names = model_data.get_attribute(ModelDataAttribute::Enum).toStringList();

			int display_name_index = display_names.indexOf(model_data.get_value().toString());
			auto type_name = type_names[display_name_index];

			rule_entry.m_value = type_name;
		}

		// Close bracket.
		if (index.column() == 3)
		{
			rule_entry.m_brackets_close = value.value<ModelData>()[0].get_value().toInt();
		}

		// Operator.
		if (index.column() == 4)
		{
			rule_entry.m_operator = EnumReflection<SchemaRuleOperator>::info(value.value<ModelData>()[0].get_value().toInt()).m_type;
		}

		auto rule_data = m_internal->m_rule->get();
		rule_data.m_entries[index.row()] = rule_entry;
		m_internal->m_rule->set(rule_data);

		emit dataChanged(this->index(index.row(), 0), this->index(index.row(), columnCount() - 1));
		return true;
	}



	//================================================================================
	// Schema Slots
	//================================================================================

	void SchemaRulePropertiesModel::slot_rule_to_be_removed(int index)
	{
		if (m_internal->m_schema == nullptr)
		{
			return;
		}

		auto rule = m_internal->m_schema->rules()[index];
		if (rule == m_internal->m_rule)
		{
			clear();
		}
	}

	void SchemaRulePropertiesModel::slot_rule_modified(int index)
	{
		if (m_internal->m_schema == nullptr)
		{
			return;
		}

		auto rule = m_internal->m_schema->rules()[index];
		if (rule == m_internal->m_rule)
		{
			emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, columnCount() - 1));
		}
	}
}