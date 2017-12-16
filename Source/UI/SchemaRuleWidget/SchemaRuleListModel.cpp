// Project includes
#include "UI/SchemaRuleWidget/SchemaRuleListModel.h"
#include "Data/Schema/Schema.h"
#include "Data/Settings.h"
#include "Utility/ModelData/ModelData.h"
#include "EditorInterface.h"

// Qt includes
#include <QCollator>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct SchemaRuleListModel::Internal
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

	SchemaRuleListModel::SchemaRuleListModel(const EditorInterface& editor_interface, QObject* parent)
		: QAbstractItemModel(parent)
		, m_internal(std::make_unique<Internal>(editor_interface))
	{
	}

	SchemaRuleListModel::~SchemaRuleListModel()
	{
	}



	//================================================================================
	// Data
	//================================================================================

	void SchemaRuleListModel::set_schema(SchemaPtr schema)
	{
		clear();

		beginResetModel();
		
		m_internal->m_schema = schema;

		connect(&schema->rules(), &SchemaRules::signal_to_be_added, this, &SchemaRuleListModel::slot_rule_to_be_added);
		connect(&schema->rules(), &SchemaRules::signal_added, this, &SchemaRuleListModel::slot_rule_added);
		connect(&schema->rules(), &SchemaRules::signal_to_be_removed, this, &SchemaRuleListModel::slot_rule_to_be_removed);
		connect(&schema->rules(), &SchemaRules::signal_removed, this, &SchemaRuleListModel::slot_rule_removed);
		connect(&schema->rules(), &SchemaRules::signal_modified, this, &SchemaRuleListModel::slot_rule_modified);

		endResetModel();
	}

	void SchemaRuleListModel::clear()
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

	QModelIndex SchemaRuleListModel::index(int row, int column, const QModelIndex& /*parent*/) const
	{
		return createIndex(row, column, row);
	}

	QModelIndex SchemaRuleListModel::parent(const QModelIndex& /*child*/) const
	{
		return QModelIndex();
	}

	int SchemaRuleListModel::rowCount(const QModelIndex& parent) const
	{
		return (m_internal->m_schema != nullptr && !parent.isValid() ? m_internal->m_schema->rules().get().size() : 0);
	}

	int SchemaRuleListModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 1;
	}

	Qt::ItemFlags SchemaRuleListModel::flags(const QModelIndex& /*index*/) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	}

	QVariant SchemaRuleListModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
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

	QVariant SchemaRuleListModel::data(const QModelIndex& index, int role) const
	{
		auto rule = m_internal->m_schema->rules()[index.row()];

		if (index.column() == 0)
		{
			if (role == Qt::DisplayRole)
			{
				return rule->get().m_name;
			}

			if (role == Qt::ForegroundRole && m_internal->m_editor_interface.get_settings().get().m_editor_show_unused_rules)
			{
				auto items = m_internal->m_schema->items().get();
				bool items_found = std::any_of(items.begin(), items.end(), [rule] (SchemaItemCPtr item)
				{
					return (item->get().m_rule == rule);
				});

				auto regions = m_internal->m_schema->regions().get();
				bool regions_found = std::any_of(regions.begin(), regions.end(), [rule] (SchemaRegionCPtr region)
				{
					return (region->get().m_rule == rule);
				});

				auto rules = m_internal->m_schema->rules().get();
				bool rules_found = std::any_of(rules.begin(), rules.end(), [rule] (SchemaRuleCPtr schema_rule)
				{
					auto entries = schema_rule->get().m_entries;
					return std::any_of(entries.begin(), entries.end(), [rule] (const SchemaRuleEntry& entry)
					{
						return (entry.m_type == SchemaRuleType::SchemaRule && entry.m_value == rule->get().m_name);
					});
				});

				return (!items_found && !regions_found && !rules_found ? QColor(128, 128, 128) : QVariant());
			}

			if (role == ModelDataRole)
			{
				ModelData data;
				data << ModelDataEntry(rule->get().m_name);

				return QVariant::fromValue(data);
			}
		}

		return QVariant();
	}

	bool SchemaRuleListModel::setData(const QModelIndex& index, const QVariant& value, int role)
	{
		auto rule = m_internal->m_schema->rules()[index.row()];

		if (index.column() == 0)
		{
			if (role == ModelDataRole)
			{
				auto name = value.value<ModelData>()[0].get_value().toString();
				if (m_internal->m_schema->rules().find(name) != nullptr)
				{
					name = rule->get().m_name;
				}

				auto data = rule->get();
				data.m_name = name;
				rule->set(data);

				emit dataChanged(index, index);

				return true;
			}
		}
		
		return false;
	}



	//================================================================================
	// Schema Slots
	//================================================================================

	void SchemaRuleListModel::slot_rule_to_be_added(int index)
	{
		beginInsertRows(QModelIndex(), index, index);
	}

	void SchemaRuleListModel::slot_rule_added(int /*index*/)
	{
		endInsertRows();
	}

	void SchemaRuleListModel::slot_rule_to_be_removed(int index)
	{
		beginRemoveRows(QModelIndex(), index, index);
	}

	void SchemaRuleListModel::slot_rule_removed(int /*index*/)
	{
		endRemoveRows();
	}

	void SchemaRuleListModel::slot_rule_modified(int index)
	{
		emit dataChanged(this->index(index, 0), this->index(index, columnCount() - 1), { Qt::DisplayRole });
	}
}


namespace LTTPMapTracker
{
	//================================================================================
	// Proxy Model
	//================================================================================

	bool SchemaRuleListProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
	{
		QCollator collator;
		collator.setNumericMode(true);

		return collator(source_left.data().toString(), source_right.data().toString());
	}
}