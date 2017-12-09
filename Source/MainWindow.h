#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

// Project includes
#include "EditorTypeInfo.h"
#include "Utility/Result.h"

// Qt includes
#include <QMainWindow>
#include <QVector>

// Stdlib includes
#include <memory>

// Forward declarations
class QGraphicsItem;

namespace LTTPMapTracker
{
	class MapSceneItem;
	struct SettingsDiff;
} 


namespace LTTPMapTracker
{
	// Types
	//--------------------------------------------------------------------------------

	enum class TrackerMode
	{
		Schema,
		Run
	};


	// Main Window
	//--------------------------------------------------------------------------------

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		// Construction & Destruction
						MainWindow							(QWidget* parent = nullptr);
						~MainWindow							();

		// Configuration
		Result			load_configuration					();
		Result			load_configuration					(QString filename);
		Result			save_configuration					();
		Result			save_configuration_as				();
		Result			close_configuration					();

		void			set_configuration					(ConfigurationPtr configuration);
		void			clear_configuration					();

		// Schema
		Result			load_schema							();
		Result			load_schema							(QString filename);
		Result			save_schema							();
		Result			save_schema_as						();
		Result			close_schema						();

		void			set_schema							(SchemaPtr schema);
		void			clear_schema						();

		// Instance
		Result			load_instance						();
		Result			load_instance						(QString filename);
		Result			save_instance						();
		Result			save_instance_as					();
		Result			close_instance						();

		bool			start_instance						();
		bool			stop_instance						();

		void			set_instance						(InstancePtr instance);
		void			clear_instance						();

	protected:
		// Qt Events
		virtual void	showEvent							(QShowEvent* event)		override;
		virtual void	closeEvent							(QCloseEvent* event)	override;
		virtual void	timerEvent							(QTimerEvent* event)	override;

	private slots:
		// Menu Slots
		void			slot_menu_file_exit					();
		void			slot_menu_edit_preferences			();

		// Widget Slots
		void			slot_map_selection_changed			(QVector<QGraphicsItem*> items);
		void			slot_schema_item_selection_changed	(SchemaItemPtr schema_item);

		// Settings Slots
		void			slot_settings_changed				(const SettingsDiff& diff);

	private:
		// Helpers
		void			set_tracker_mode					(TrackerMode mode);
		void			update_title						();
		void			update_settings						();

		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif