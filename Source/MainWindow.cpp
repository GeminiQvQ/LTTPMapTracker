// Project includes
#include "MainWindow.h"
#include "UI/ConfigurationWidget/ConfigurationWidget.h"
#include "UI/MapWidget/Items/MapSceneItemSchemaItem.h"
#include "UI/MapWidget/MapScene.h"
#include "UI/MapWidget/MapWidget.h"
#include "UI/ProgressLocationWidget/ProgressLocationWidget.h"
#include "UI/ProgressItemWidget/ProgressItemWidget.h"
#include "UI/SchemaItemWidget/SchemaItemWidget.h"
#include "UI/SchemaRegionWidget/SchemaRegionWidget.h"
#include "UI/SchemaRuleWidget/SchemaRuleWidget.h"
#include "UI/SettingsWidget/SettingsWidget.h"
#include "UI/StartupWidget/StartupWidget.h"
#include "Data/Instance/Instance.h"
#include "Data/Schema/Schema.h"
#include "Data/DataModel.h"
#include "Data/Settings.h"
#include "Utility/File.h"
#include "Utility/WidgetState/WidgetStateManager.h"
#include "Utility/WindowManager.h"
#include "EditorInterface.h"

// Qt includes
#include <QCloseEvent>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include "ui_MainWindow.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct MainWindow::Internal
	{
		struct EditorInterfaceImpl : public EditorInterface
		{
		public:
			EditorInterfaceImpl(Internal& internal) : m_internal(internal) {}
			
			virtual Settings&				get_settings	()			override { return m_internal.m_settings;	}
			virtual const Settings&			get_settings	()	const	override { return m_internal.m_settings;	}
			virtual DataModel&				get_data_model	()			override { return m_internal.m_data_model;	}
			virtual const DataModel&		get_data_model	()	const	override { return m_internal.m_data_model;	}

		private:
			Internal& m_internal;
		};

		using WindowManagerPtr = std::unique_ptr<WindowManager>;

		EditorInterfaceImpl		m_editor_interface;
		Settings				m_settings;
		DataModel				m_data_model;
		WidgetStateManager		m_widget_state_manager;
		WindowManagerPtr		m_window_manager;

		MapWidget*				m_map_widget;
		ConfigurationWidget*	m_configuration_widget;
		SchemaItemWidget*		m_schema_item_widget;
		SchemaRegionWidget*		m_schema_region_widget;
		SchemaRuleWidget*		m_schema_rule_widget;
		ProgressItemWidget*		m_progress_item_widget;
		ProgressLocationWidget*	m_progress_location_widget;

		Ui::MainWindow			m_ui;
		QVector<QAction*>		m_schema_menu_actions;
		QVector<QAction*>		m_schema_toolbar_actions;
		QVector<QAction*>		m_instance_menu_actions;
		QVector<QAction*>		m_instance_toolbar_actions;

		QAction*				m_start_action;
		QAction*				m_stop_action;
		int						m_window_config_schema;
		int						m_window_config_run;
		int						m_timer_id;

		ConfigurationPtr		m_configuration;

		Internal()
			: m_editor_interface(*this)
			, m_map_widget(nullptr)
			, m_configuration_widget(nullptr)
			, m_schema_item_widget(nullptr)
			, m_schema_region_widget(nullptr)
			, m_schema_rule_widget(nullptr)
			, m_progress_item_widget(nullptr)
			, m_progress_location_widget(nullptr)
			, m_start_action(nullptr)
			, m_stop_action(nullptr)
			, m_window_config_schema(0)
			, m_window_config_run(0)
			, m_timer_id(0)
			, m_configuration(std::make_shared<Configuration>(m_data_model))
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	MainWindow::MainWindow(QWidget* parent)
		: QMainWindow(parent)
		, m_internal(std::make_unique<Internal>())
	{
		m_internal->m_ui.setupUi(this);

		// Widget.
		setWindowIcon(QIcon(":/App/Icon"));
		setCentralWidget(nullptr);
		setDockNestingEnabled(true);
		layout()->setSizeConstraint(QLayout::SetNoConstraint);
		m_internal->m_widget_state_manager.register_widget(*this);

		QFile fh(":/App/Style");
		if (fh.open(QIODevice::ReadOnly))
		{
			setStyleSheet(fh.readAll());
			fh.close();
		}

		// Data.
		auto data_result = m_internal->m_data_model.load();
		for (auto it = data_result.begin(); it != data_result.end(); ++it)
		{
			report_result(*it, this, it.key());
		}

		// Settings.
		m_internal->m_settings.load();
		connect(&m_internal->m_settings, &Settings::signal_changed, this, &MainWindow::slot_settings_changed, Qt::QueuedConnection);

		// Menu.
		auto menu_file = menuBar()->addMenu(tr("&File"));
		m_internal->m_schema_menu_actions << menu_file->addAction("Load Configuration...", this, (Result(MainWindow::*)())&MainWindow::load_configuration, Qt::CTRL | Qt::Key_O);
		m_internal->m_schema_menu_actions << menu_file->addAction("Save Configuration", this, &MainWindow::save_configuration, Qt::CTRL | Qt::Key_S);
		m_internal->m_schema_menu_actions << menu_file->addAction("Save Configuration As...", this, &MainWindow::save_configuration_as, Qt::CTRL | Qt::ALT | Qt::Key_S);
		m_internal->m_schema_menu_actions << menu_file->addAction("Close Configuration", this, &MainWindow::close_configuration, Qt::CTRL | Qt::Key_F4);
		menu_file->addSeparator();
		m_internal->m_schema_menu_actions << menu_file->addAction("Load Schema...", this, (Result(MainWindow::*)())&MainWindow::load_schema);
		m_internal->m_schema_menu_actions << menu_file->addAction("Save Schema", this, &MainWindow::save_schema);
		m_internal->m_schema_menu_actions << menu_file->addAction("Save Schema As...", this, &MainWindow::save_schema_as);
		m_internal->m_schema_menu_actions << menu_file->addAction("Close Schema", this, &MainWindow::close_schema);
		menu_file->addSeparator();
		m_internal->m_instance_menu_actions << menu_file->addAction("Load Instance...", this, (Result(MainWindow::*)())&MainWindow::load_instance);
		m_internal->m_instance_menu_actions << menu_file->addAction("Save Instance", this, &MainWindow::save_instance);
		m_internal->m_instance_menu_actions << menu_file->addAction("Save Instance As...", this, &MainWindow::save_instance_as);
		menu_file->addSeparator();
		menu_file->addAction("Exit", this, &MainWindow::slot_menu_file_exit, Qt::ALT | Qt::Key_F4);

		auto menu_edit = menuBar()->addMenu(tr("&Edit"));
		menu_edit->addAction("Preferences...", this, &MainWindow::slot_menu_edit_preferences);

		auto menu_run = menuBar()->addMenu(tr("&Run"));
		m_internal->m_start_action = menu_run->addAction(QIcon(":/Status/Play"), "Start", this, &MainWindow::start_instance, Qt::Key_F5);
		m_internal->m_stop_action = menu_run->addAction(QIcon(":/Status/Stop"), "Stop", this, &MainWindow::stop_instance, Qt::SHIFT | Qt::Key_F5);

		auto menu_view = menuBar()->addMenu(tr("&View"));

		// Widgets.
		m_internal->m_window_manager = std::make_unique<WindowManager>(*this, *menu_view, [] () { return std::make_unique<QSettings>("Data/Settings.ini", QSettings::IniFormat); });
		m_internal->m_window_config_schema = m_internal->m_window_manager->add_configuration("Schema");
		m_internal->m_window_config_run = m_internal->m_window_manager->add_configuration("Run");

		m_internal->m_map_widget = new MapWidget(m_internal->m_editor_interface, this);
		connect(m_internal->m_map_widget, &MapWidget::signal_selection_changed, this, &MainWindow::slot_map_selection_changed);
		m_internal->m_window_manager->add_dockable_widget(*m_internal->m_map_widget, Qt::RightDockWidgetArea, {m_internal->m_window_config_schema, m_internal->m_window_config_run});
		
		m_internal->m_configuration_widget = new ConfigurationWidget(m_internal->m_editor_interface, this);
		m_internal->m_window_manager->add_dockable_widget(*m_internal->m_configuration_widget, Qt::LeftDockWidgetArea, {m_internal->m_window_config_schema});

		m_internal->m_schema_item_widget = new SchemaItemWidget(this);
		connect(m_internal->m_schema_item_widget, &SchemaItemWidget::signal_selection_changed, this, &MainWindow::slot_schema_item_selection_changed);
		m_internal->m_window_manager->add_dockable_widget(*m_internal->m_schema_item_widget, Qt::LeftDockWidgetArea, {m_internal->m_window_config_schema});

		m_internal->m_schema_region_widget = new SchemaRegionWidget(m_internal->m_editor_interface, this);
		m_internal->m_window_manager->add_dockable_widget(*m_internal->m_schema_region_widget, Qt::LeftDockWidgetArea, {m_internal->m_window_config_schema});

		m_internal->m_schema_rule_widget = new SchemaRuleWidget(m_internal->m_editor_interface, this);
		m_internal->m_window_manager->add_dockable_widget(*m_internal->m_schema_rule_widget, Qt::LeftDockWidgetArea, {m_internal->m_window_config_schema});

		m_internal->m_progress_item_widget = new ProgressItemWidget(m_internal->m_editor_interface, this);
		m_internal->m_window_manager->add_dockable_widget(*m_internal->m_progress_item_widget, Qt::LeftDockWidgetArea, {m_internal->m_window_config_run});

		m_internal->m_progress_location_widget = new ProgressLocationWidget(m_internal->m_editor_interface, this);
		m_internal->m_window_manager->add_dockable_widget(*m_internal->m_progress_location_widget, Qt::LeftDockWidgetArea, {m_internal->m_window_config_run});

		// Toolbar.
		auto toolbar = m_internal->m_ui.toolbar;
		m_internal->m_schema_toolbar_actions << toolbar->addAction(QIcon(":/File/Open"), "Load Configuration", this, (Result(MainWindow::*)())&MainWindow::load_configuration);
		m_internal->m_schema_toolbar_actions << toolbar->addAction(QIcon(":/File/Save"), "Save Configuration", this, &MainWindow::save_configuration);
		m_internal->m_schema_toolbar_actions << toolbar->addAction(QIcon(":/File/SaveAs"), "Save Configuration As...", this, &MainWindow::save_configuration_as);
		m_internal->m_schema_toolbar_actions << toolbar->addAction(QIcon(":/File/Close"), "Close Configuration", this, &MainWindow::close_configuration);
		m_internal->m_instance_toolbar_actions << toolbar->addAction(QIcon(":/File/Open"), "Load Instance", this, (Result(MainWindow::*)())&MainWindow::load_instance);
		m_internal->m_instance_toolbar_actions << toolbar->addAction(QIcon(":/File/Save"), "Save Instance", this, &MainWindow::save_instance);
		m_internal->m_instance_toolbar_actions << toolbar->addAction(QIcon(":/File/SaveAs"), "Save Instance As...", this, &MainWindow::save_instance_as);
		m_internal->m_instance_toolbar_actions << toolbar->addAction(QIcon(":/File/Close"), "Close Instance", this, &MainWindow::close_instance);
		toolbar->clear();
		toolbar->addSeparator();
		toolbar->addAction(m_internal->m_stop_action);
		toolbar->addAction(m_internal->m_start_action);
		
		// Initial state.
		clear_configuration();

		QSettings settings("Data/Settings.ini", QSettings::IniFormat);

		settings.beginGroup("Window");
		m_internal->m_widget_state_manager.load(settings);
		settings.endGroup();

		settings.beginGroup("Settings/App");

		auto configuration_filename = settings.value("ConfigurationFile").toString();
		if (!configuration_filename.isEmpty())
		{
			load_configuration(configuration_filename);
		}

		auto schema_filename = settings.value("SchemaFile").toString();
		if (!schema_filename.isEmpty())
		{
			load_schema(schema_filename);
		}

		settings.endGroup();

		m_internal->m_timer_id = startTimer(m_internal->m_settings.get().m_general_autosave_interval * 1000);
	}

	MainWindow::~MainWindow()
	{
		QSettings settings("Data/Settings.ini", QSettings::IniFormat);

		settings.beginGroup("Window");
		m_internal->m_widget_state_manager.save(settings);
		settings.endGroup();
	}
	


	//================================================================================
	// Configuration
	//================================================================================

	Result MainWindow::load_configuration()
	{
		auto filename = QFileDialog::getOpenFileName(this, "Load Configuration", QApplication::applicationDirPath() + "/Data/Configurations/", "Configuration Files (*.configuration.json)", nullptr, QFileDialog::DontResolveSymlinks);
		if (filename.isEmpty())
		{
			return Result(ResultType::Error, QString());
		}

		return load_configuration(filename);
	}

	Result MainWindow::load_configuration(QString filename)
	{
		auto configuration = std::make_shared<Configuration>(m_internal->m_data_model);
		auto load_result = configuration->load(filename);
		if (!load_result)
		{
			report_result(load_result, this, "Load Result");
			return load_result;
		}

		auto close_result = close_configuration();
		if (!close_result)
		{
			report_result(close_result, this, "Close Result");
			return close_result;
		}

		set_configuration(configuration);

		return Result();
	}

	Result MainWindow::save_configuration()
	{
		Result result;

		if (!m_internal->m_configuration->get_filename().isEmpty())
		{
			result = m_internal->m_configuration->save();
			report_result(result, this, "Save Result");
		}
		else
		{
			result = save_configuration_as();
		}

		auto schema_result = save_schema();
		if (!schema_result)
		{
			return schema_result;
		}

		return result;
	}

	Result MainWindow::save_configuration_as()
	{
		auto filename = QFileDialog::getSaveFileName(this, "Save Configuration", QApplication::applicationDirPath() + "/Data/Configurations/", "Configuration Files (*.configuration.json)", nullptr, QFileDialog::DontResolveSymlinks);
		if (filename.isEmpty())
		{
			return Result();
		}

		auto save_result = m_internal->m_configuration->save(filename);
		if (!save_result)
		{
			report_result(save_result, this, "Save Result");
			return save_result;
		}

		auto schema_result = save_schema();
		if (!schema_result)
		{
			return schema_result;
		}

		return Result();
	}

	Result MainWindow::close_configuration()
	{
		auto schema_result = close_schema();
		if (!schema_result)
		{
			return schema_result;
		}

		set_configuration(std::make_shared<Configuration>(m_internal->m_data_model));

		return Result();
	}

	void MainWindow::set_configuration(ConfigurationPtr configuration)
	{
		m_internal->m_configuration = configuration;
		m_internal->m_configuration_widget->set_configuration(configuration);
		set_schema(configuration->get().m_schema);
		
		m_internal->m_settings.reset();
	}

	void MainWindow::clear_configuration()
	{
		set_configuration(std::make_shared<Configuration>(m_internal->m_data_model));
	}



	//================================================================================
	// Schema
	//================================================================================

	Result MainWindow::load_schema()
	{
		auto filename = QFileDialog::getOpenFileName(this, "Load Schema", QApplication::applicationDirPath() + "/Data/Schemas/", "Schema Files (*.schema.json)", nullptr, QFileDialog::DontResolveSymlinks);
		if (filename.isEmpty())
		{
			return Result();
		}

		return load_schema(filename);
	}

	Result MainWindow::load_schema(QString filename)
	{
		auto close_result = close_schema();
		if (!close_result)
		{
			return close_result;
		}

		auto schema = std::make_shared<Schema>();
		auto load_result = schema->load(filename);
		report_result(load_result, this, "Load Result");

		if (!load_result)
		{
			return load_result;
		}

		set_schema(schema);
		
		return Result();
	}

	Result MainWindow::save_schema()
	{
		Result result;

		if (!m_internal->m_configuration->get().m_schema->get_filename().isEmpty())
		{
			result = m_internal->m_configuration->get().m_schema->save(m_internal->m_configuration->get().m_schema->get_filename());
			report_result(result, this, "Save Result");
		}
		else
		{
			result = save_schema_as();
		}

		return result;
	}

	Result MainWindow::save_schema_as()
	{
		auto filename = QFileDialog::getSaveFileName(this, "Save Schema As", QApplication::applicationDirPath() + "/Data/Schemas/", "Schema Files (*.schema.json)", nullptr, QFileDialog::DontResolveSymlinks);
		if (filename.isEmpty())
		{
			return Result();
		}

		auto result = m_internal->m_configuration->get().m_schema->save(filename);
		report_result(result, this, "Save Result");

		return result;
	}

	Result MainWindow::close_schema()
	{
		if (m_internal->m_configuration->get().m_schema->is_dirty())
		{
			auto a = QMessageBox::question(this, "LTTP Map Editor", "Do you want to save changes to the current schema?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
			
			if (a == QMessageBox::Save)
			{
				auto result = save_schema();
				if (!result)
				{
					return result;
				}
			}

			if (a == QMessageBox::Cancel)
			{
				return Result(false, QString());
			}
		}

		clear_schema();

		return Result();
	}

	void MainWindow::set_schema(SchemaPtr schema)
	{
		if (m_internal->m_configuration->get().m_schema != nullptr)
		{
			m_internal->m_configuration->get().m_schema->disconnect(this);
		}

		m_internal->m_map_widget->set_schema(schema);
		m_internal->m_schema_item_widget->set_schema(schema);
		m_internal->m_schema_region_widget->set_schema(schema);
		m_internal->m_schema_rule_widget->set_schema(schema);

		auto configuration_data = m_internal->m_configuration->get();
		configuration_data.m_schema = schema;
		m_internal->m_configuration->set(configuration_data);

		connect(schema.get(), &Schema::signal_dirty_state_changed, this, &MainWindow::update_title);

		update_title();
	}

	void MainWindow::clear_schema()
	{
		m_internal->m_map_widget->clear_schema();
		m_internal->m_schema_item_widget->clear_schema();
		m_internal->m_schema_region_widget->clear_schema();
		m_internal->m_schema_rule_widget->clear_schema();
		set_schema(std::make_shared<Schema>());
	}



	//================================================================================
	// Instance
	//================================================================================

	Result MainWindow::load_instance()
	{
		auto filename = QFileDialog::getOpenFileName(this, "Load Instance", QApplication::applicationDirPath() + "/Data/Instances/", "Instance Files (*.instance.json)", nullptr, QFileDialog::DontResolveSymlinks);
		if (filename.isEmpty())
		{
			return Result();
		}

		return load_instance(filename);
	}

	Result MainWindow::load_instance(QString filename)
	{
		auto close_result = close_instance();
		if (!close_result)
		{
			return close_result;
		}

		auto instance = std::make_shared<Instance>(m_internal->m_data_model, m_internal->m_configuration->get().m_schema);
		auto load_result = instance->load(filename);
		report_result(load_result, this, "Load Result");

		if (!load_result)
		{
			return load_result;
		}

		set_instance(instance);
		
		return Result();
	}

	Result MainWindow::save_instance()
	{
		Result result;

		if (!m_internal->m_configuration->get().m_instance->get_filename().isEmpty())
		{
			result = m_internal->m_configuration->get().m_instance->save();
			report_result(result, this, "Save Result");
		}
		else
		{
			result = save_instance_as();
		}

		return result;
	}

	Result MainWindow::save_instance_as()
	{
		auto filename = QFileDialog::getSaveFileName(this, "Save Instance As", QApplication::applicationDirPath() + "/Data/Instances/", "Instance Files (*.instance.json)", nullptr, QFileDialog::DontResolveSymlinks);
		if (filename.isEmpty())
		{
			return Result();
		}

		auto result = m_internal->m_configuration->get().m_instance->save(filename);
		report_result(result, this, "Save Result");

		return result;
	}

	Result MainWindow::close_instance()
	{
		if (m_internal->m_configuration->get().m_instance->is_dirty())
		{
			auto a = QMessageBox::question(this, "LTTP Map Editor", "Do you want to save changes to the current instance?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
			
			if (a == QMessageBox::Save)
			{
				auto result = save_instance();
				if (!result)
				{
					return result;
				}
			}

			if (a == QMessageBox::Cancel)
			{
				return Result(false, QString());
			}
		}

		auto configuration_data = m_internal->m_configuration->get();
		configuration_data.m_instance = std::make_shared<Instance>(m_internal->m_data_model, m_internal->m_configuration->get().m_schema);
		m_internal->m_configuration->set(configuration_data);
		
		return Result();
	}

	bool MainWindow::start_instance()
	{
		if (m_internal->m_configuration->get().m_instance == nullptr)
		{
			auto create_result = m_internal->m_configuration->create_instance();
			report_result(create_result, this, "Create Instance Result");

			set_instance(m_internal->m_configuration->get().m_instance);

			set_tracker_mode(TrackerMode::Run);
		}

		return true;
	}

	bool MainWindow::stop_instance()
	{
		if (m_internal->m_configuration->get().m_instance != nullptr)
		{
			if (!close_instance())
			{
				return false;
			}

			clear_instance();

			set_tracker_mode(TrackerMode::Schema);
		}

		return true;
	}

	void MainWindow::set_instance(InstancePtr instance)
	{
		if (m_internal->m_configuration->get().m_instance != nullptr)
		{
			m_internal->m_configuration->get().m_instance->disconnect(this);
		}

		m_internal->m_map_widget->clear_schema();
		m_internal->m_map_widget->set_instance(instance);
		m_internal->m_progress_item_widget->set_instance(instance);
		m_internal->m_progress_location_widget->set_instance(instance);
		
		auto configuration_data = m_internal->m_configuration->get();
		configuration_data.m_instance = instance;
		m_internal->m_configuration->set(configuration_data);

		connect(instance.get(), &Instance::signal_dirty_state_changed, this, &MainWindow::update_title);

		update_title();
	}

	void MainWindow::clear_instance()
	{
		m_internal->m_map_widget->clear_instance();
		m_internal->m_map_widget->set_schema(m_internal->m_configuration->get().m_schema);
		m_internal->m_progress_item_widget->clear_instance();
		m_internal->m_progress_location_widget->clear_instance();

		m_internal->m_configuration->destroy_instance();

		update_title();
	}



	//================================================================================
	// Qt Events
	//================================================================================

	void MainWindow::showEvent(QShowEvent* /*event*/)
	{
		bool initialised = property("Initialized").toBool();
		if (!initialised)
		{
			set_tracker_mode(TrackerMode::Schema);
			setProperty("Initialized", true);

			if (m_internal->m_settings.get().m_general_startup_show)
			{
				StartupWidget startup_widget(m_internal->m_editor_interface, this);
				
				if (startup_widget.exec() == QDialog::Accepted)
				{
					if (load_configuration(startup_widget.get_selected_configuration()) &&
						m_internal->m_settings.get().m_general_startup_autorun)
					{
						start_instance();
					}
				}
			}
		}
	}
	
	void MainWindow::closeEvent(QCloseEvent* event)
	{
		QSettings settings("Data/Settings.ini", QSettings::IniFormat);

		settings.beginGroup("Settings/App");
		settings.setValue("ConfigurationFile", get_relative_path(m_internal->m_configuration->get_filename()));
		settings.setValue("SchemaFile", get_relative_path(m_internal->m_configuration->get().m_schema->get_filename()));
		settings.endGroup();

		if (!stop_instance() ||
			!close_schema())
		{
			event->ignore();
			return;
		}

		m_internal->m_window_manager->save();
	}

	void MainWindow::timerEvent(QTimerEvent* /*event*/)
	{
		auto instance = m_internal->m_configuration->get().m_instance;

		if (instance != nullptr)
		{
			if (m_internal->m_settings.get().m_general_autosave_temp)
			{
				instance->save_auto();
			}
			
			if (m_internal->m_settings.get().m_general_autosave_main &&
				!instance->get_filename().isEmpty())
			{
				instance->save();
			}
		}
	}



	//================================================================================
	// Menu Slots
	//================================================================================

	void MainWindow::slot_menu_file_exit()
	{
		close();
	}

	void MainWindow::slot_menu_edit_preferences()
	{
		Settings settings;
		settings.load();

		SettingsWidget w(settings, this);
		w.exec();

		m_internal->m_settings.set(settings.get());
		m_internal->m_settings.save();
	}



	//================================================================================
	// Widget Slots
	//================================================================================

	void MainWindow::slot_map_selection_changed(QVector<QGraphicsItem*> items)
	{
		// Schema items.
		m_internal->m_schema_item_widget->select_item(nullptr);

		QVector<MapSceneItemSchemaItem*> schema_items;
		for (auto item : items)
		{
			if (static_cast<const MapScene*>(item->scene())->get_item_type(*item) == MapSceneItemType::SchemaItem)
			{
				schema_items << static_cast<MapSceneItemSchemaItem*>(item);
			}
		}
		
		if (schema_items.size() == 1)
		{
			auto schema_item = static_cast<MapSceneItemSchemaItem*>(schema_items[0])->get_schema_item();
			m_internal->m_schema_item_widget->select_item(schema_item);
		}
	}

	void MainWindow::slot_schema_item_selection_changed(SchemaItemPtr schema_item)
	{
		m_internal->m_schema_region_widget->select_region(nullptr);
		m_internal->m_schema_rule_widget->select_rule(nullptr);

		if (schema_item != nullptr)
		{
			auto schema_region = schema_item->get().m_region;
			if (schema_region != nullptr)
			{
				m_internal->m_schema_region_widget->select_region(schema_region);

				if (schema_region->get().m_rule != nullptr)
				{
					m_internal->m_schema_rule_widget->select_rule(schema_region->get().m_rule);
				}
			}

			if (schema_item->get().m_rule != nullptr)
			{
				m_internal->m_schema_rule_widget->select_rule(schema_item->get().m_rule);
			}

			m_internal->m_map_widget->blockSignals(true);
			m_internal->m_map_widget->select_schema_item(schema_item);
			m_internal->m_map_widget->blockSignals(false);
		}
	}



	//================================================================================
	// Settings Slots
	//================================================================================

	void MainWindow::slot_settings_changed(const SettingsDiff& diff)
	{
		if (diff.has_change(&SettingsData::m_general_autosave_interval))
		{
			killTimer(m_internal->m_timer_id);
			m_internal->m_timer_id = startTimer(diff.m_new.m_general_autosave_interval * 1000);
		}

		update_settings();
	}

	

	//================================================================================
	// Helpers
	//================================================================================

	void MainWindow::set_tracker_mode(TrackerMode mode)
	{
		auto add_actions = (mode == TrackerMode::Schema ? m_internal->m_schema_toolbar_actions : m_internal->m_instance_toolbar_actions);
		auto remove_actions = (mode == TrackerMode::Schema ? m_internal->m_instance_toolbar_actions : m_internal->m_schema_toolbar_actions);

		for (auto action : remove_actions)
		{
			m_internal->m_ui.toolbar->removeAction(action);
		}

		auto separator = m_internal->m_ui.toolbar->actions().front();
		for (auto action : add_actions)
		{
			m_internal->m_ui.toolbar->insertAction(separator, action);
		}

		m_internal->m_configuration_widget->setEnabled(mode == TrackerMode::Schema);
		m_internal->m_schema_item_widget->setEnabled(mode == TrackerMode::Schema);
		m_internal->m_schema_region_widget->setEnabled(mode == TrackerMode::Schema);
		m_internal->m_schema_rule_widget->setEnabled(mode == TrackerMode::Schema);

		m_internal->m_progress_item_widget->setEnabled(mode == TrackerMode::Run);
		m_internal->m_progress_location_widget->setEnabled(mode == TrackerMode::Run);

		m_internal->m_start_action->setEnabled(mode == TrackerMode::Schema);
		m_internal->m_stop_action->setEnabled(mode == TrackerMode::Run);

		m_internal->m_window_manager->enable_configuration(mode == TrackerMode::Schema ? m_internal->m_window_config_schema : m_internal->m_window_config_run);
	}

	void MainWindow::update_title()
	{
		QString mode;
		QString filename;
		bool dirty;

		if (m_internal->m_configuration->get().m_instance != nullptr)
		{
			mode = "Instance";
			filename = m_internal->m_configuration->get().m_instance->get_filename().section("/", -1, -1);
			dirty = m_internal->m_configuration->get().m_instance->is_dirty();
		}
		else
		{
			mode = "Schema";
			filename = m_internal->m_configuration->get().m_schema->get_filename().section("/", -1, -1);
			dirty = m_internal->m_configuration->get().m_schema->is_dirty();
		}

		setWindowTitle(QString("LTTP Map Tracker - %1 - %2%3").arg(mode).arg(!filename.isEmpty() ? filename : "<Untitled>").arg(dirty ? " (*)" : ""));
	}

	void MainWindow::update_settings()
	{
		if (!m_internal->m_configuration->get().m_settings_overrides.isEmpty())
		{
			Settings settings;
			settings.set(m_internal->m_settings.get());
			settings.load(m_internal->m_configuration->get().m_settings_overrides);

			m_internal->m_settings.disconnect(this);
			m_internal->m_settings.set(settings.get());
			connect(&m_internal->m_settings, &Settings::signal_changed, this, &MainWindow::slot_settings_changed, Qt::QueuedConnection);
		}
	}
}