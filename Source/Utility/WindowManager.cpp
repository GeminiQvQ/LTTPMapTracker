// Project includes
#include "Utility/WindowManager.h"

// Qt includes
#include <QDockWidget>
#include <QHash>
#include <QMainWindow>
#include <QMenu>
#include <QSettings>


namespace Utility
{
	//================================================================================
	// Internal
	//================================================================================

	struct WindowManager::Internal
	{
		using DockWidgetMap = QMap<int, QVector<QDockWidget*>>;

		QMainWindow&			m_main_window;
		QMenu&					m_menu;
		SettingsCreator			m_settings_creator;

		QStringList				m_configurations;
		QVector<QDockWidget*>	m_dock_widgets;

		QByteArray				m_default_state;
		int						m_configuration;

		Internal(QMainWindow& main_window, QMenu& menu, SettingsCreator settings_creator)
			: m_main_window(main_window)
			, m_menu(menu)
			, m_settings_creator(settings_creator)
			, m_configurations({"default"})
			, m_configuration(0)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	WindowManager::WindowManager(QMainWindow& main_window, QMenu& menu, SettingsCreator settings_creator)
		: m_internal(std::make_unique<Internal>(main_window, menu, settings_creator))
	{
		load();
	}

	WindowManager::~WindowManager()
	{
	}



	//================================================================================
	// Configuration Management
	//================================================================================

	int WindowManager::add_configuration(QString name)
	{
		Q_ASSERT(!m_internal->m_configurations.contains(name));
		int id = m_internal->m_configurations.size();
		m_internal->m_configurations << name;
		return id;
	}

	void WindowManager::enable_configuration(int configuration)
	{
		Q_ASSERT(configuration >= 0 && configuration < m_internal->m_configurations.size());

		if (configuration != m_internal->m_configuration)
		{
			save();

			m_internal->m_configuration = configuration;

			load();
		}
	}



	//================================================================================
	// Widget Management
	//================================================================================

	void WindowManager::add_dockable_widget(QWidget& widget, Qt::DockWidgetArea dock_area, QVector<int> configurations)
	{
		auto dock_widget = new QDockWidget(widget.windowTitle());
		dock_widget->setWidget(&widget);
		dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
		dock_widget->setObjectName(widget.windowTitle());
		m_internal->m_main_window.addDockWidget(dock_area, dock_widget);
		m_internal->m_dock_widgets << dock_widget;

		m_internal->m_menu.clear();

		for (auto dock_widget_ : m_internal->m_dock_widgets)
		{
			m_internal->m_menu.addAction(dock_widget_->toggleViewAction());
		}

		m_internal->m_menu.addSeparator();
		m_internal->m_menu.addAction("Reset Windows", [this] () { reset(); });
	}



	//================================================================================
	// State
	//================================================================================

	void WindowManager::save()
	{
		auto settings = m_internal->m_settings_creator();
		settings->beginGroup("Window");
		settings->setValue("Pos", m_internal->m_main_window.pos());
		settings->setValue("Size", m_internal->m_main_window.size());
		settings->beginGroup(m_internal->m_configurations[m_internal->m_configuration]);
		settings->setValue("State", m_internal->m_main_window.saveState());
	}

	void WindowManager::load()
	{
		auto settings = m_internal->m_settings_creator();
		settings->beginGroup("Window");
		m_internal->m_main_window.move(settings->value("Pos", m_internal->m_main_window.pos()).toPoint());
		m_internal->m_main_window.resize(settings->value("Size", m_internal->m_main_window.size()).toSize());
		settings->beginGroup(m_internal->m_configurations[m_internal->m_configuration]);
		m_internal->m_main_window.restoreState(settings->value("State").toByteArray());
		settings->endGroup();
	}

	void WindowManager::reset()
	{
		auto settings = m_internal->m_settings_creator();
		settings->beginGroup("Window");
		m_internal->m_main_window.restoreGeometry(settings->value("DefaultGeometry").toByteArray());
		settings->beginGroup(m_internal->m_configurations[m_internal->m_configuration]);
		m_internal->m_main_window.restoreState(settings->value("DefaultState").toByteArray());
	}
}