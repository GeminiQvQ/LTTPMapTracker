#ifndef SETTINGS_H
#define SETTINGS_H

// Qt includes
#include <QColor>
#include <QObject>


namespace LTTPMapTracker
{
	// Types
	//--------------------------------------------------------------------------------

	struct SettingsData
	{
		bool	m_general_startup_show;
		bool	m_general_startup_autorun;

		bool	m_general_autosave_temp;
		bool	m_general_autosave_main;
		int		m_general_autosave_interval;

		QString m_general_layout_instance_items;
		QString m_general_layout_instance_locations;
		QString m_general_layout_progress_items;
		QString m_general_layout_progress_locations;

		bool	m_editor_show_unused_regions;
		bool	m_editor_show_unused_rules;

		float	m_map_background_opacity;
		float	m_map_connection_thickness;
		QColor	m_map_connection_color;

		int		m_map_item_size;
		float	m_map_item_opacity_cleared;
		QColor	m_map_item_color_base;
		QColor	m_map_item_color_inaccessible;
		QColor	m_map_item_color_item_requirement;
		QColor	m_map_item_color_item_requirement_fulfilled;
		QColor	m_map_item_color_location;
		QColor	m_map_item_color_location_requirement;
		QColor	m_map_item_color_location_requirement_fulfilled;
		QString	m_map_item_entity_item_requirement;

		SettingsData();
	};

	struct SettingsDiff
	{
		SettingsData m_old;
		SettingsData m_new;

		template <typename T>
		bool has_change(T SettingsData::* data) const
		{
			return (m_old.*data != m_new.*data);
		}
	};


	// Settings
	//--------------------------------------------------------------------------------

	class Settings : public QObject
	{
		Q_OBJECT

	public:
		// Construction & Destruction
							Settings		(QObject* parent = nullptr);

		// Load & Save
		void				load			();
		void				load			(QString filename);
		void				save			();
		void				reset			();
		
		// Data
		const SettingsData& get				() const;
		void				set				(const SettingsData& data);

	signals:
		// Signals
		void				signal_changed	(const SettingsDiff& diff);

	private:
		SettingsData		m_data;
	};
}

Q_DECLARE_METATYPE(LTTPMapTracker::SettingsDiff);

#endif