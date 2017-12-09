// Project includes
#include "UI/SchemaRuleWidget/SchemaRuleWidget.h"
#include "UI/SchemaRuleWidget/SchemaRuleListModel.h"
#include "UI/SchemaRuleWidget/SchemaRulePropertiesModel.h"
#include "Data/Schema/Schema.h"
#include "Utility/ModelData/ModelDataDelegate.h"

// Qt includes
#include <QLayout>
#include <QListView>
#include <QMenu>
#include <QSplitter>
#include <QTreeView>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct SchemaRuleWidget::Internal
	{
		SchemaRuleListModel			m_list_model;
		SchemaRuleListProxyModel	m_list_proxy;
		QTreeView*					m_list_view;
		SchemaRulePropertiesModel	m_properties_model;
		QTreeView*					m_properties_view;

		SchemaPtr					m_schema;
		SchemaRulePtr				m_rule;

		Internal(EditorInterface& editor_interface)
			: m_list_model(editor_interface)
			, m_list_view(nullptr)
			, m_properties_model(editor_interface)
			, m_properties_view(nullptr)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SchemaRuleWidget::SchemaRuleWidget(EditorInterface& editor_interface, QWidget* parent)
		: QWidget(parent)
		, m_internal(std::make_unique<Internal>(editor_interface))
	{
		// Widget.
		setWindowTitle("Schema Rule Editor");
		
		auto layout = new QVBoxLayout();
		setLayout(layout);

		auto splitter = new QSplitter(Qt::Horizontal);
		splitter->setObjectName("SchemaRuleWidgetSplitter");
		splitter->setChildrenCollapsible(false);
		layout->addWidget(splitter);

		// List view.
		m_internal->m_list_proxy.setSourceModel(&m_internal->m_list_model);
		m_internal->m_list_view = new QTreeView();
		m_internal->m_list_view->setObjectName("SchemaRuleWidgetListView");
		m_internal->m_list_view->setModel(&m_internal->m_list_proxy);
		m_internal->m_list_view->setAlternatingRowColors(true);
		m_internal->m_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_internal->m_list_view->setIndentation(0);
		m_internal->m_list_view->setItemDelegate(new ModelDataDelegate());
		m_internal->m_list_view->setContextMenuPolicy(Qt::CustomContextMenu);
		m_internal->m_list_view->setSortingEnabled(true);
		m_internal->m_list_view->sortByColumn(0, Qt::AscendingOrder);
		connect(m_internal->m_list_view, &QListView::customContextMenuRequested, this, &SchemaRuleWidget::slot_list_menu);
		connect(m_internal->m_list_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SchemaRuleWidget::slot_list_selection_changed);
		splitter->addWidget(m_internal->m_list_view);

		// Properties view.
		m_internal->m_properties_view = new QTreeView();
		m_internal->m_properties_view->setObjectName("SchemaRuleWidgetPropertiesView");
		m_internal->m_properties_view->setModel(&m_internal->m_properties_model);
		m_internal->m_properties_view->setIndentation(0);
		m_internal->m_properties_view->setAlternatingRowColors(true);
		m_internal->m_properties_view->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_internal->m_properties_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_internal->m_properties_view->setItemDelegate(new ModelDataDelegate());
		m_internal->m_properties_view->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_internal->m_properties_view, &QListView::customContextMenuRequested, this, &SchemaRuleWidget::slot_properties_menu);
		splitter->addWidget(m_internal->m_properties_view);
	}

	SchemaRuleWidget::~SchemaRuleWidget()
	{
	}



	//================================================================================
	// Schema
	//================================================================================

	void SchemaRuleWidget::set_schema(SchemaPtr schema)
	{
		m_internal->m_schema = schema;
		m_internal->m_list_model.set_schema(schema);
	}

	void SchemaRuleWidget::clear_schema()
	{
		m_internal->m_schema = nullptr;
		m_internal->m_list_model.clear();
	}



	//================================================================================
	// Selection
	//================================================================================

	void SchemaRuleWidget::select_rule(SchemaRulePtr rule)
	{
		auto index = m_internal->m_list_proxy.mapFromSource(m_internal->m_list_model.index(m_internal->m_schema->rules().get().indexOf(rule), 0));

		m_internal->m_list_view->clearSelection();
		m_internal->m_list_view->setCurrentIndex(index);
	}



	//================================================================================
	// UI Slots
	//================================================================================

	void SchemaRuleWidget::slot_list_menu()
	{
		bool has_schema	= (m_internal->m_schema != nullptr);
		bool has_selection = !m_internal->m_list_view->selectionModel()->selection().isEmpty();

		QMenu menu(this);
		menu.addAction("Add Rule", this, &SchemaRuleWidget::slot_add_rule)->setEnabled(has_schema);
		menu.addAction("Remove Rules", this, &SchemaRuleWidget::slot_remove_rule)->setEnabled(has_selection);
		menu.exec(QCursor::pos());
	}

	void SchemaRuleWidget::slot_list_selection_changed()
	{
		auto indices = m_internal->m_list_view->selectionModel()->selectedRows();

		if (indices.size() == 1)
		{
			m_internal->m_rule = m_internal->m_schema->rules()[m_internal->m_list_proxy.mapToSource(indices[0]).row()];
			m_internal->m_properties_model.set_rule(m_internal->m_schema, m_internal->m_rule);
		}
		else
		{
			m_internal->m_rule = nullptr;
			m_internal->m_properties_model.clear();
		}
	}

	void SchemaRuleWidget::slot_properties_menu()
	{
		bool has_rule	= (m_internal->m_rule != nullptr);
		bool has_selection = !m_internal->m_properties_view->selectionModel()->selection().isEmpty();

		QMenu menu(this);
		menu.addAction("Add Rule Entry", this, &SchemaRuleWidget::slot_add_rule_entry)->setEnabled(has_rule);
		menu.addAction("Insert Rule Entry (Before)", this, &SchemaRuleWidget::slot_insert_rule_entry_before)->setEnabled(has_rule && has_selection);
		menu.addAction("Insert Rule Entry (After)", this, &SchemaRuleWidget::slot_insert_rule_entry_after)->setEnabled(has_rule && has_selection);
		menu.addAction("Remove Rule Entries", this, &SchemaRuleWidget::slot_remove_rule_entry)->setEnabled(has_selection);
		menu.exec(QCursor::pos());
	}

	//--------------------------------------------------------------------------------

	void SchemaRuleWidget::slot_add_rule()
	{
		m_internal->m_schema->rules().add();
	}

	void SchemaRuleWidget::slot_remove_rule()
	{
		QVector<SchemaRulePtr> rules;

		for (auto index : m_internal->m_list_view->selectionModel()->selectedRows())
		{
			rules << m_internal->m_schema->rules()[m_internal->m_list_proxy.mapToSource(index).row()];
		}

		for (auto rule : rules)
		{
			m_internal->m_schema->rules().remove(rule);
		}
	}

	//--------------------------------------------------------------------------------

	void SchemaRuleWidget::slot_add_rule_entry()
	{
		m_internal->m_properties_model.insert_entry();
	}

	void SchemaRuleWidget::slot_insert_rule_entry_before()
	{
		auto index = m_internal->m_properties_view->selectionModel()->selectedRows()[0];
		m_internal->m_properties_model.insert_entry(index.row());
	}

	void SchemaRuleWidget::slot_insert_rule_entry_after()
	{
		auto index = m_internal->m_properties_view->selectionModel()->selectedRows()[0];
		m_internal->m_properties_model.insert_entry(index.row() + 1);
	}

	void SchemaRuleWidget::slot_remove_rule_entry()
	{
		QVector<int> indices;

		for (auto index : m_internal->m_properties_view->selectionModel()->selectedRows())
		{
			indices << index.row();
		}

		m_internal->m_properties_model.remove_entries(indices);
	}
}