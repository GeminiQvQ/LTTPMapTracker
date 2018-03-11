// Project includes
#include "UI/StartupWidget/StartupWidget.h"
#include "UI/StartupWidget/StartupModel.h"
#include "Data/Settings.h"
#include "EditorInterface.h"

// Qt includes
#include "ui_StartupWidget.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct StartupWidget::Internal
	{
		EditorInterface&	m_editor_interface;
		StartupModel		m_model;

		Ui::StartupWidget	m_ui;

		Internal(EditorInterface& editor_interface)
			: m_editor_interface(editor_interface)
			, m_model(editor_interface.get_data_model())
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	StartupWidget::StartupWidget(EditorInterface& editor_interface, QWidget* parent)
		: QDialog(parent)
		, m_internal(std::make_unique<Internal>(editor_interface))
	{
		// UI.
		m_internal->m_ui.setupUi(this);
		
		m_internal->m_ui.show->setChecked(editor_interface.get_settings().get().m_general_startup_show);
		connect(m_internal->m_ui.show, &QCheckBox::toggled, this, &StartupWidget::slot_show_changed);

		m_internal->m_ui.autorun->setChecked(editor_interface.get_settings().get().m_general_startup_autorun);
		connect(m_internal->m_ui.autorun, &QCheckBox::toggled, this, &StartupWidget::slot_autorun_changed);

		connect(m_internal->m_ui.button_ok, &QPushButton::clicked, this, &StartupWidget::accept);
		connect(m_internal->m_ui.button_cancel, &QPushButton::clicked, this, &StartupWidget::reject);

		// View.
		m_internal->m_ui.list->setModel(&m_internal->m_model);
		connect(m_internal->m_ui.list, &QListView::doubleClicked, this, &StartupWidget::accept);
		connect(m_internal->m_ui.list->selectionModel(), &QItemSelectionModel::selectionChanged, this, &StartupWidget::validate_ui);

		// Initial state.
		validate_ui();
	}

	StartupWidget::~StartupWidget()
	{
	}



	//================================================================================
	// Accessors
	//================================================================================

	QString StartupWidget::get_selected_configuration() const
	{
		return m_internal->m_ui.list->currentIndex().data(Qt::UserRole).toString();
	}



	//================================================================================
	// UI Slots
	//================================================================================

	void StartupWidget::slot_show_changed(bool enabled)
	{
		auto data = m_internal->m_editor_interface.get_settings().get();
		data.m_general_startup_show = enabled;
		m_internal->m_editor_interface.get_settings().set(data);
		m_internal->m_editor_interface.get_settings().save();
	}

	void StartupWidget::slot_autorun_changed(bool enabled)
	{
		auto data = m_internal->m_editor_interface.get_settings().get();
		data.m_general_startup_autorun = enabled;
		m_internal->m_editor_interface.get_settings().set(data);
		m_internal->m_editor_interface.get_settings().save();
	}



	//================================================================================
	// Helpers
	//================================================================================

	void StartupWidget::validate_ui()
	{
		m_internal->m_ui.button_ok->setEnabled(!get_selected_configuration().isEmpty());
	}
}