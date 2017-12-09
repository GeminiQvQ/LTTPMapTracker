// Project includes
#include "UI/SchemaRegionWidget/SchemaRegionWidget.h"
#include "UI/SchemaRegionWidget/SchemaRegionListModel.h"
#include "UI/SchemaRegionWidget/SchemaRegionPropertiesModel.h"
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

	struct SchemaRegionWidget::Internal
	{
		SchemaRegionListModel		m_list_model;
		SchemaRegionListProxyModel	m_list_proxy;
		QTreeView*					m_list_view;
		SchemaRegionPropertiesModel	m_properties_model;
		QTreeView*					m_properties_view;

		SchemaPtr					m_schema;

		Internal(const EditorInterface& editor_interface)
			: m_list_model(editor_interface)
			, m_list_view(nullptr)
			, m_properties_view(nullptr)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SchemaRegionWidget::SchemaRegionWidget(const EditorInterface& editor_interface, QWidget* parent)
		: QWidget(parent)
		, m_internal(std::make_unique<Internal>(editor_interface))
	{
		// Widget.
		setWindowTitle("Schema Region Editor");
		
		auto layout = new QVBoxLayout();
		setLayout(layout);

		auto splitter = new QSplitter(Qt::Vertical);
		splitter->setObjectName("SchemaRegionWidgetSplitter");
		splitter->setChildrenCollapsible(false);
		layout->addWidget(splitter);

		// List view.
		m_internal->m_list_proxy.setSourceModel(&m_internal->m_list_model);
		m_internal->m_list_view = new QTreeView();
		m_internal->m_list_view->setObjectName("SchemaRegionWidgetListView");
		m_internal->m_list_view->setModel(&m_internal->m_list_proxy);
		m_internal->m_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_internal->m_list_view->setContextMenuPolicy(Qt::CustomContextMenu);
		m_internal->m_list_view->setAlternatingRowColors(true);
		m_internal->m_list_view->setIndentation(0);
		m_internal->m_list_view->setSortingEnabled(true);
		m_internal->m_list_view->sortByColumn(0, Qt::AscendingOrder);
		connect(m_internal->m_list_view, &QListView::customContextMenuRequested, this, &SchemaRegionWidget::slot_list_menu);
		connect(m_internal->m_list_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SchemaRegionWidget::slot_list_selection_changed);
		splitter->addWidget(m_internal->m_list_view);

		// Properties view.
		m_internal->m_properties_view = new QTreeView();
		m_internal->m_properties_view->setObjectName("SchemaRegionWidgetPropertiesView");
		m_internal->m_properties_view->setModel(&m_internal->m_properties_model);
		m_internal->m_properties_view->setEditTriggers(QAbstractItemView::AllEditTriggers);
		m_internal->m_properties_view->setItemDelegate(new ModelDataDelegate());
		m_internal->m_properties_view->setAlternatingRowColors(true);
		m_internal->m_properties_view->setIndentation(0);
		splitter->addWidget(m_internal->m_properties_view);
	}

	SchemaRegionWidget::~SchemaRegionWidget()
	{
	}



	//================================================================================
	// Schema
	//================================================================================

	void SchemaRegionWidget::set_schema(SchemaPtr schema)
	{
		m_internal->m_schema = schema;
		m_internal->m_list_model.set_schema(schema);
	}

	void SchemaRegionWidget::clear_schema()
	{
		m_internal->m_schema = nullptr;
		m_internal->m_list_model.clear();
	}



	//================================================================================
	// Selection
	//================================================================================

	void SchemaRegionWidget::select_region(SchemaRegionPtr region)
	{
		auto index = m_internal->m_list_proxy.mapFromSource(m_internal->m_list_model.index(m_internal->m_schema->regions().get().indexOf(region), 0));

		m_internal->m_list_view->clearSelection();
		m_internal->m_list_view->setCurrentIndex(index);
	}



	//================================================================================
	// UI Slots
	//================================================================================

	void SchemaRegionWidget::slot_list_menu()
	{
		bool has_schema	= (m_internal->m_schema != nullptr);
		bool has_selection = !m_internal->m_list_view->selectionModel()->selection().isEmpty();

		QMenu menu(this);
		menu.addAction("Add Region", this, &SchemaRegionWidget::slot_add_region)->setEnabled(has_schema);
		menu.addAction("Remove Regions", this, &SchemaRegionWidget::slot_remove_region)->setEnabled(has_selection);
		menu.exec(QCursor::pos());
	}

	void SchemaRegionWidget::slot_list_selection_changed()
	{
		auto indices = m_internal->m_list_view->selectionModel()->selectedRows();

		if (indices.size() == 1)
		{
			auto region = m_internal->m_schema->regions()[m_internal->m_list_proxy.mapToSource(indices[0]).row()];
			m_internal->m_properties_model.set_region(m_internal->m_schema, region);
		}
		else
		{
			m_internal->m_properties_model.clear();
		}
	}

	//--------------------------------------------------------------------------------

	void SchemaRegionWidget::slot_add_region()
	{
		m_internal->m_schema->regions().add();
	}

	void SchemaRegionWidget::slot_remove_region()
	{
		QVector<SchemaRegionPtr> regions;

		for (auto index : m_internal->m_list_view->selectionModel()->selectedRows())
		{
			regions << m_internal->m_schema->regions()[m_internal->m_list_proxy.mapToSource(index).row()];
		}

		for (auto region : regions)
		{
			m_internal->m_schema->regions().remove(region);
		}
	}
}