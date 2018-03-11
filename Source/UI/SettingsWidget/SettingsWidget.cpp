// Project includes
#include "UI/SettingsWidget/SettingsWidget.h"
#include "Data/Settings.h"
#include "Utility/Widget/ColorButtonWidget.h"
#include "Utility/Widget/FileBrowseWidget.h"

// Qt includes
#include "ui_SettingsWidget.h"


namespace LTTPMapTracker
{
	//================================================================================
	// UI
	//================================================================================

	struct UI : public Ui::SettingsWidget
	{
		FileBrowseWidget*	general_layout_instance_items;
		FileBrowseWidget*	general_layout_instance_locations;
		FileBrowseWidget*	general_layout_progress_items;
		FileBrowseWidget*	general_layout_progress_locations;
		ColorButtonWidget*	map_item_color_base;
		ColorButtonWidget*	map_item_color_inaccessible;
		ColorButtonWidget*	map_item_color_item_requirement;
		ColorButtonWidget*	map_item_color_item_requirement_fulfilled;
		ColorButtonWidget*	map_item_color_location;
		ColorButtonWidget*	map_item_color_location_requirement;
		ColorButtonWidget*	map_item_color_location_requirement_fulfilled;
		ColorButtonWidget*	map_connection_color;
	};



	//================================================================================
	// Utility
	//================================================================================

	void settings_to_ui(const Settings& settings, UI& ui)
	{
		ui.general_startup_show->setChecked(settings.get().m_general_startup_show);
		ui.general_startup_autorun->setChecked(settings.get().m_general_startup_autorun);
		ui.general_autosave_temp->setChecked(settings.get().m_general_autosave_temp);
		ui.general_autosave_main->setChecked(settings.get().m_general_autosave_main);
		ui.general_autosave_interval->setValue(settings.get().m_general_autosave_interval);
		ui.general_layout_instance_items->set_filename(settings.get().m_general_layout_instance_items);
		ui.general_layout_instance_locations->set_filename(settings.get().m_general_layout_instance_locations);
		ui.general_layout_progress_items->set_filename(settings.get().m_general_layout_progress_items);
		ui.general_layout_progress_locations->set_filename(settings.get().m_general_layout_progress_locations);
		ui.editor_show_unused_regions->setChecked(settings.get().m_editor_show_unused_regions);
		ui.editor_show_unused_rules->setChecked(settings.get().m_editor_show_unused_rules);
		ui.map_background_opacity->setValue(settings.get().m_map_background_opacity);
		ui.map_connection_thickness->setValue(settings.get().m_map_connection_thickness);
		ui.map_connection_color->set_color(settings.get().m_map_connection_color);
		ui.map_item_size->setValue(settings.get().m_map_item_size);
		ui.map_item_cleared_opacity->setValue(settings.get().m_map_item_opacity_cleared);
		ui.map_item_color_base->set_color(settings.get().m_map_item_color_base);
		ui.map_item_color_inaccessible->set_color(settings.get().m_map_item_color_inaccessible);
		ui.map_item_color_item_requirement->set_color(settings.get().m_map_item_color_item_requirement);
		ui.map_item_color_item_requirement_fulfilled->set_color(settings.get().m_map_item_color_item_requirement_fulfilled);
		ui.map_item_color_location->set_color(settings.get().m_map_item_color_location);
		ui.map_item_color_location_requirement->set_color(settings.get().m_map_item_color_location_requirement);
		ui.map_item_color_location_requirement_fulfilled->set_color(settings.get().m_map_item_color_location_requirement_fulfilled);
		ui.map_item_entity_item_requirement->setText(settings.get().m_map_item_entity_item_requirement);
	}

	void ui_to_settings(Settings& settings, const UI& ui)
	{
		auto data = settings.get();
		data.m_general_startup_show = ui.general_startup_show->isChecked();
		data.m_general_startup_autorun = ui.general_startup_autorun->isChecked();
		data.m_general_autosave_temp = ui.general_autosave_temp->isChecked();
		data.m_general_autosave_main = ui.general_autosave_main->isChecked();
		data.m_general_autosave_interval = ui.general_autosave_interval->value();
		data.m_general_layout_instance_items = ui.general_layout_instance_items->get_filename();
		data.m_general_layout_instance_locations = ui.general_layout_instance_locations->get_filename();
		data.m_general_layout_progress_items = ui.general_layout_progress_items->get_filename();
		data.m_general_layout_progress_locations = ui.general_layout_progress_locations->get_filename();
		data.m_editor_show_unused_regions = ui.editor_show_unused_regions->isChecked();
		data.m_editor_show_unused_rules = ui.editor_show_unused_rules->isChecked();
		data.m_map_background_opacity = ui.map_background_opacity->value();
		data.m_map_connection_thickness = ui.map_connection_thickness->value();
		data.m_map_connection_color = ui.map_connection_color->get_color();
		data.m_map_item_size = ui.map_item_size->value();
		data.m_map_item_opacity_cleared = ui.map_item_cleared_opacity->value();
		data.m_map_item_color_base = ui.map_item_color_base->get_color();
		data.m_map_item_color_inaccessible = ui.map_item_color_inaccessible->get_color();
		data.m_map_item_color_item_requirement = ui.map_item_color_item_requirement->get_color();
		data.m_map_item_color_item_requirement_fulfilled = ui.map_item_color_item_requirement_fulfilled->get_color();
		data.m_map_item_color_location = ui.map_item_color_location->get_color();
		data.m_map_item_color_location_requirement = ui.map_item_color_location_requirement->get_color();
		data.m_map_item_color_location_requirement_fulfilled = ui.map_item_color_location_requirement_fulfilled->get_color();
		data.m_map_item_entity_item_requirement = ui.map_item_entity_item_requirement->text();
		settings.set(data);
	}



	//================================================================================
	// Internal
	//================================================================================

	struct SettingsWidget::Internal
	{
		Settings&	m_settings;
		UI			m_ui;

		Internal(Settings& settings)
			: m_settings(settings)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	SettingsWidget::SettingsWidget(Settings& settings, QWidget* parent)
		: QDialog(parent)
		, m_internal(std::make_unique<Internal>(settings))
	{
		m_internal->m_ui.setupUi(this);

		m_internal->m_ui.general_layout_instance_items = new FileBrowseWidget("Select Layout", "Data/Layouts/", "Layout Files (*.layout.json)");
		m_internal->m_ui.general_layout_instance_items_layout->addWidget(m_internal->m_ui.general_layout_instance_items);
		m_internal->m_ui.general_layout_instance_locations = new FileBrowseWidget("Select Layout", "Data/Layouts/", "Layout Files (*.layout.json)");
		m_internal->m_ui.general_layout_instance_locations_layout->addWidget(m_internal->m_ui.general_layout_instance_locations);
		m_internal->m_ui.general_layout_progress_items = new FileBrowseWidget("Select Layout", "Data/Layouts/", "Layout Files (*.layout.json)");
		m_internal->m_ui.general_layout_progress_items_layout->addWidget(m_internal->m_ui.general_layout_progress_items);
		m_internal->m_ui.general_layout_progress_locations = new FileBrowseWidget("Select Layout", "Data/Layouts/", "Layout Files (*.layout.json)");
		m_internal->m_ui.general_layout_progress_locations_layout->addWidget(m_internal->m_ui.general_layout_progress_locations);
		m_internal->m_ui.map_connection_color = new ColorButtonWidget();
		m_internal->m_ui.map_connection_color_layout->addWidget(m_internal->m_ui.map_connection_color);
		m_internal->m_ui.map_item_color_base = new ColorButtonWidget();
		m_internal->m_ui.map_item_color_base_layout->addWidget(m_internal->m_ui.map_item_color_base);
		m_internal->m_ui.map_item_color_inaccessible = new ColorButtonWidget();
		m_internal->m_ui.map_item_color_inaccessible_layout->addWidget(m_internal->m_ui.map_item_color_inaccessible);
		m_internal->m_ui.map_item_color_item_requirement = new ColorButtonWidget();
		m_internal->m_ui.map_item_color_item_requirement_layout->addWidget(m_internal->m_ui.map_item_color_item_requirement);
		m_internal->m_ui.map_item_color_item_requirement_fulfilled = new ColorButtonWidget();
		m_internal->m_ui.map_item_color_item_requirement_fulfilled_layout->addWidget(m_internal->m_ui.map_item_color_item_requirement_fulfilled);
		m_internal->m_ui.map_item_color_location = new ColorButtonWidget();
		m_internal->m_ui.map_item_color_location_layout->addWidget(m_internal->m_ui.map_item_color_location);
		m_internal->m_ui.map_item_color_location_requirement = new ColorButtonWidget();
		m_internal->m_ui.map_item_color_location_requirement_layout->addWidget(m_internal->m_ui.map_item_color_location_requirement);
		m_internal->m_ui.map_item_color_location_requirement_fulfilled = new ColorButtonWidget();
		m_internal->m_ui.map_item_color_location_requirement_fulfilled_layout->addWidget(m_internal->m_ui.map_item_color_location_requirement_fulfilled);
		
		settings_to_ui(m_internal->m_settings, m_internal->m_ui);

		connect(m_internal->m_ui.button_ok, &QPushButton::clicked, this, &SettingsWidget::slot_ok);
		connect(m_internal->m_ui.button_cancel, &QPushButton::clicked, this, &SettingsWidget::reject);
	}

	SettingsWidget::~SettingsWidget()
	{
	}



	//================================================================================
	// UI Slots
	//================================================================================

	void SettingsWidget::slot_ok()
	{
		ui_to_settings(m_internal->m_settings, m_internal->m_ui);
		accept();
	}
}