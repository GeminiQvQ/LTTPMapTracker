// Project includes
#include "Data/Settings.h"

// Qt includes
#include <QSettings>


namespace LTTPMapTracker
{
	//================================================================================
	// Settings Data
	//================================================================================

	SettingsData::SettingsData()
		: m_general_startup_show(true)
		, m_general_startup_autorun(true)
		, m_general_autosave_temp(true)
		, m_general_autosave_main(false)
		, m_general_autosave_interval(60)
		, m_general_layout_instance_items("Data/Layouts/InstanceItems.layout.json")
		, m_general_layout_instance_locations("Data/Layouts/InstanceLocations.layout.json")
		, m_general_layout_progress_items("Data/Layouts/ProgressItems.layout.json")
		, m_general_layout_progress_locations("Data/Layouts/ProgressLocations.layout.json")
		, m_editor_show_unused_regions(false)
		, m_editor_show_unused_rules(false)
		, m_map_background_opacity(0.5f)
		, m_map_connection_thickness(3.0f)
		, m_map_connection_color(130, 180, 220, 255)
		, m_map_item_size(24)
		, m_map_item_opacity_cleared(0.1f)
		, m_map_item_color_base(200, 200, 200, 255)
		, m_map_item_color_inaccessible(200, 100, 100, 96)
		, m_map_item_color_item_requirement(192, 96, 96, 255)
		, m_map_item_color_item_requirement_fulfilled(96, 192, 96, 255)
		, m_map_item_color_location(220, 210, 70, 255)
		, m_map_item_color_location_requirement(220, 210, 70, 255)
		, m_map_item_color_location_requirement_fulfilled(0, 92, 255, 255)
		, m_map_item_entity_item_requirement("Plus")
	{
	}



	//================================================================================
	// Load & Save
	//================================================================================

	void Settings::load()
	{
		load("Data/Settings.ini");
	}

	void Settings::load(QString filename)
	{
		QSettings settings(filename, QSettings::IniFormat);
		settings.beginGroup("Settings");

		settings.beginGroup("General");
		m_data.m_general_startup_show = settings.value("StartupShow", m_data.m_general_startup_show).toBool();
		m_data.m_general_startup_autorun = settings.value("StartupAutorun", m_data.m_general_startup_autorun).toBool();
		m_data.m_general_autosave_temp = settings.value("AutosaveTemp", m_data.m_general_autosave_temp).toBool();
		m_data.m_general_autosave_main = settings.value("AutosaveMain", m_data.m_general_autosave_main).toBool();
		m_data.m_general_autosave_interval = settings.value("AutosaveInterval", m_data.m_general_autosave_interval).toInt();
		m_data.m_general_layout_instance_items = settings.value("LayoutInstanceItems", m_data.m_general_layout_instance_items).toString();
		m_data.m_general_layout_instance_locations = settings.value("LayoutInstanceLocations", m_data.m_general_layout_instance_locations).toString();
		m_data.m_general_layout_progress_items = settings.value("LayoutProgressItems", m_data.m_general_layout_progress_items).toString();
		m_data.m_general_layout_progress_locations = settings.value("LayoutProgressLocations", m_data.m_general_layout_progress_locations).toString();
		settings.endGroup();

		settings.beginGroup("Editor");
		m_data.m_editor_show_unused_regions = settings.value("ShowUnusedRegions", m_data.m_editor_show_unused_regions).toBool();
		m_data.m_editor_show_unused_rules = settings.value("ShowUnusedRules", m_data.m_editor_show_unused_rules).toBool();
		settings.endGroup();

		settings.beginGroup("Map");
		m_data.m_map_background_opacity = settings.value("BackgroundOpacity", m_data.m_map_background_opacity).toFloat();
		m_data.m_map_connection_thickness = settings.value("ConnectionThickness", m_data.m_map_connection_thickness).toFloat();
		m_data.m_map_connection_color = settings.value("ConnectionColor", m_data.m_map_connection_color).toString();
		settings.endGroup();

		settings.beginGroup("MapItem");
		m_data.m_map_item_size = settings.value("Size", m_data.m_map_item_size).toFloat();
		m_data.m_map_item_opacity_cleared = settings.value("OpacityCleared", m_data.m_map_item_opacity_cleared).toFloat();
		m_data.m_map_item_color_base = settings.value("ColorBase", m_data.m_map_item_color_base).toString();
		m_data.m_map_item_color_inaccessible = settings.value("ColorInaccessible", m_data.m_map_item_color_inaccessible).toString();
		m_data.m_map_item_color_item_requirement = settings.value("ColorItemRequirement", m_data.m_map_item_color_item_requirement).toString();
		m_data.m_map_item_color_item_requirement_fulfilled = settings.value("ColorItemRequirementFulfilled", m_data.m_map_item_color_item_requirement_fulfilled).toString();
		m_data.m_map_item_color_location = settings.value("ColorLocation", m_data.m_map_item_color_location).toString();
		m_data.m_map_item_color_location_requirement = settings.value("ColorLocationRequirement", m_data.m_map_item_color_location_requirement).toString();
		m_data.m_map_item_color_location_requirement_fulfilled = settings.value("ColorLocationRequirementFulfilled", m_data.m_map_item_color_location_requirement_fulfilled).toString();
		m_data.m_map_item_entity_item_requirement = settings.value("EntityItemRequirement", m_data.m_map_item_entity_item_requirement).toString();
		settings.endGroup();
	}

	void Settings::save()
	{
		QSettings settings("Data/Settings.ini", QSettings::IniFormat);
		settings.beginGroup("Settings");

		settings.beginGroup("General");
		settings.setValue("StartupShow", m_data.m_general_startup_show);
		settings.setValue("StartupAutorun", m_data.m_general_startup_autorun);
		settings.setValue("AutosaveTemp", m_data.m_general_autosave_temp);
		settings.setValue("AutosaveMain", m_data.m_general_autosave_main);
		settings.setValue("AutosaveInterval", m_data.m_general_autosave_interval);
		settings.setValue("LayoutInstanceItems", m_data.m_general_layout_instance_items);
		settings.setValue("LayoutInstanceLocations", m_data.m_general_layout_instance_locations);
		settings.setValue("LayoutProgressItems", m_data.m_general_layout_progress_items);
		settings.setValue("LayoutProgressLocations", m_data.m_general_layout_progress_locations);
		settings.endGroup();

		settings.beginGroup("Editor");
		settings.setValue("ShowUnusedRegions", m_data.m_editor_show_unused_regions);
		settings.setValue("ShowUnusedRules", m_data.m_editor_show_unused_rules);
		settings.endGroup();

		settings.beginGroup("Map");
		settings.setValue("BackgroundOpacity", (double)m_data.m_map_background_opacity);
		settings.setValue("ConnectionThickness", (double)m_data.m_map_connection_thickness);
		settings.setValue("ConnectionColor", m_data.m_map_connection_color.name(QColor::HexArgb));
		settings.endGroup();

		settings.beginGroup("MapItem");
		settings.setValue("Size", (double)m_data.m_map_item_size);
		settings.setValue("OpacityCleared", (double)m_data.m_map_item_opacity_cleared);
		settings.setValue("ColorBase", m_data.m_map_item_color_base.name(QColor::HexArgb));
		settings.setValue("ColorInaccessible", m_data.m_map_item_color_inaccessible.name(QColor::HexArgb));
		settings.setValue("ColorItemRequirement", m_data.m_map_item_color_item_requirement.name(QColor::HexArgb));
		settings.setValue("ColorItemRequirementFulfilled", m_data.m_map_item_color_item_requirement_fulfilled.name(QColor::HexArgb));
		settings.setValue("ColorLocation", m_data.m_map_item_color_location.name(QColor::HexArgb));
		settings.setValue("ColorLocationRequirement", m_data.m_map_item_color_location_requirement.name(QColor::HexArgb));
		settings.setValue("ColorLocationRequirementFulfilled", m_data.m_map_item_color_location_requirement_fulfilled.name(QColor::HexArgb));
		settings.setValue("EntityItemRequirement", m_data.m_map_item_entity_item_requirement);
		settings.endGroup();
	}

	void Settings::reset()
	{
		Settings settings;
		settings.load();
		set(settings.get());
	}



	//================================================================================
	// Construction & Destruction
	//================================================================================

	Settings::Settings(QObject* parent)
		: QObject(parent)
	{
	}



	//================================================================================
	// Data
	//================================================================================

	const SettingsData& Settings::get() const
	{
		return m_data;
	}

	void Settings::set(const SettingsData& data)
	{
		auto old = m_data;
		m_data = data;
		emit signal_changed({old, data});
	}
}