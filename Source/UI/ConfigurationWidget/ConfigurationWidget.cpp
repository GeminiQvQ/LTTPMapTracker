// Project includes
#include "UI/ConfigurationWidget/ConfigurationWidget.h"
#include "UI/ConfigurationWidget/ConfigurationModel.h"
#include "Utility/ModelData/ModelDataDelegate.h"

// Qt includes
#include <QLayout>
#include <QTreeView>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct ConfigurationWidget::Internal
	{
		ConfigurationModel	m_model;
		QTreeView*			m_view;

		Internal(EditorInterface& editor_interface)
			: m_model(editor_interface)
			, m_view(nullptr)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	ConfigurationWidget::ConfigurationWidget(EditorInterface& editor_interface, QWidget* parent)
		: QWidget(parent)
		, m_internal(std::make_unique<Internal>(editor_interface))
	{
		// Widget.
		setWindowTitle("Configuration Editor");
		
		auto layout = new QVBoxLayout();
		setLayout(layout);

		// View.
		m_internal->m_view = new QTreeView();
		m_internal->m_view->setObjectName("ConfigurationView");
		m_internal->m_view->setModel(&m_internal->m_model);
		m_internal->m_view->setEditTriggers(QAbstractItemView::AllEditTriggers);
		m_internal->m_view->setItemDelegate(new ModelDataDelegate());
		m_internal->m_view->setAlternatingRowColors(true);
		m_internal->m_view->setIndentation(0);
		layout->addWidget(m_internal->m_view);
	}

	ConfigurationWidget::~ConfigurationWidget()
	{
	}



	//================================================================================
	// Configuration
	//================================================================================

	void ConfigurationWidget::set_configuration(ConfigurationPtr configuration)
	{
		m_internal->m_model.set_configuration(configuration);
	}

	void ConfigurationWidget::clear_configuration()
	{
		m_internal->m_model.clear_configuration();
	}
}