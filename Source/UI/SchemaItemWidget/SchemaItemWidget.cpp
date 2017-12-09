// Project includes
#include "UI/SchemaItemWidget/SchemaItemWidget.h"
#include "UI/SchemaItemWidget/SchemaItemListModel.h"
#include "UI/SchemaItemWidget/SchemaItemPropertiesModel.h"
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

	struct SchemaItemWidget::Internal
	{
		SchemaItemListModel			m_list_model;
		SchemaItemListProxyModel	m_list_proxy;
		QTreeView*					m_list_view;
		SchemaItemPropertiesModel	m_properties_model;
		QTreeView*					m_properties_view;

		SchemaPtr					m_schema;

		Internal()
			: m_list_view(nullptr)
			, m_properties_view(nullptr)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SchemaItemWidget::SchemaItemWidget(QWidget* parent)
		: QWidget(parent)
		, m_internal(std::make_unique<Internal>())
	{
		// Widget.
		setWindowTitle("Schema Item Editor");

		auto layout = new QVBoxLayout();
		setLayout(layout);

		auto splitter = new QSplitter(Qt::Vertical);
		splitter->setObjectName("SchemaItemWidgetSplitter");
		splitter->setChildrenCollapsible(false);
		layout->addWidget(splitter);

		// List view.
		m_internal->m_list_proxy.setSourceModel(&m_internal->m_list_model);
		m_internal->m_list_view = new QTreeView();
		m_internal->m_list_view->setObjectName("SchemaItemWidgetListView");
		m_internal->m_list_view->setModel(&m_internal->m_list_proxy);
		m_internal->m_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_internal->m_list_view->setContextMenuPolicy(Qt::CustomContextMenu);
		m_internal->m_list_view->setAlternatingRowColors(true);
		m_internal->m_list_view->setIndentation(0);
		m_internal->m_list_view->setSortingEnabled(true);
		m_internal->m_list_view->sortByColumn(0, Qt::AscendingOrder);
		connect(m_internal->m_list_view, &QTreeView::customContextMenuRequested, this, &SchemaItemWidget::slot_list_menu);
		connect(m_internal->m_list_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SchemaItemWidget::slot_list_selection_changed);
		splitter->addWidget(m_internal->m_list_view);

		// Properties view.
		m_internal->m_properties_view = new QTreeView();
		m_internal->m_properties_view->setObjectName("SchemaItemWidgetPropertiesView");
		m_internal->m_properties_view->setModel(&m_internal->m_properties_model);
		m_internal->m_properties_view->setEditTriggers(QAbstractItemView::AllEditTriggers);
		m_internal->m_properties_view->setItemDelegate(new ModelDataDelegate());
		m_internal->m_properties_view->setAlternatingRowColors(true);
		m_internal->m_properties_view->setIndentation(0);
		splitter->addWidget(m_internal->m_properties_view);
	}

	SchemaItemWidget::~SchemaItemWidget()
	{
	}



	//================================================================================
	// Schema
	//================================================================================

	void SchemaItemWidget::set_schema(SchemaPtr schema)
	{
		m_internal->m_schema = schema;
		m_internal->m_list_model.set_schema(schema);
	}

	void SchemaItemWidget::clear_schema()
	{
		m_internal->m_schema = nullptr;
		m_internal->m_list_model.clear();
	}



	//================================================================================
	// Selection
	//================================================================================

	void SchemaItemWidget::select_item(SchemaItemPtr item)
	{
		auto index = m_internal->m_list_proxy.mapFromSource(m_internal->m_list_model.index(m_internal->m_schema->items().get().indexOf(item), 0));

		m_internal->m_list_view->clearSelection();
		m_internal->m_list_view->setCurrentIndex(index);
	}



	//================================================================================
	// UI Slots
	//================================================================================

	void SchemaItemWidget::slot_list_menu()
	{
		bool has_schema = (m_internal->m_schema != nullptr);
		bool has_selection = !m_internal->m_list_view->selectionModel()->selection().isEmpty();

		QMenu menu(this);
		menu.addAction("Add Item", this, &SchemaItemWidget::slot_add_item)->setEnabled(has_schema);
		menu.addAction("Remove Items", this, &SchemaItemWidget::slot_remove_item)->setEnabled(has_selection);
		menu.exec(QCursor::pos());
	}

	void SchemaItemWidget::slot_list_selection_changed()
	{
		auto indices = m_internal->m_list_view->selectionModel()->selectedRows();

		if (indices.size() == 1)
		{
			auto item = m_internal->m_schema->items()[m_internal->m_list_proxy.mapToSource(indices[0]).row()];
			m_internal->m_properties_model.set_item(m_internal->m_schema, item);
			emit signal_selection_changed(item);
		}
		else
		{
			m_internal->m_properties_model.clear();
			emit signal_selection_changed(nullptr);
		}
	}

	//--------------------------------------------------------------------------------

	void SchemaItemWidget::slot_add_item()
	{
		m_internal->m_schema->items().add();
	}

	void SchemaItemWidget::slot_remove_item()
	{
		QVector<SchemaItemPtr> items;

		for (auto index : m_internal->m_list_view->selectionModel()->selectedRows())
		{
			items << m_internal->m_schema->items()[m_internal->m_list_proxy.mapToSource(index).row()];
		}

		for (auto item : items)
		{
			m_internal->m_schema->items().remove(item);
		}
	}
}