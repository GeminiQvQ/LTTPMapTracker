// Project includes
#include "UI/MapWidget/MapWidget.h"
#include "UI/MapWidget/MapEntityWidget.h"
#include "UI/MapWidget/MapScene.h"
#include "UI/MapWidget/MapView.h"
#include "UI/MapWidget/Items/MapSceneItemSchemaItem.h"
#include "UI/MapWidget/Items/MapSceneItemInstanceItem.h"
#include "Data/Instance/Instance.h"
#include "Data/Schema/Schema.h"
#include "Data/Settings.h"
#include "Utility/EnumReflection.h"
#include "Utility/File.h"
#include "EditorInterface.h"

// Qt includes
#include <QApplication>
#include <QIcon>
#include <QLayout>
#include <QList>
#include <QMenu>
#include <QTabBar>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct MapWidget::Internal
	{
		QTabBar*			m_tab_bar;
		QVector<MapScene*>	m_scenes;
		MapView*			m_view;
		
		SchemaPtr			m_schema;
		InstancePtr			m_instance;

		MapEntityWidget*	m_ew_items;
		MapEntityWidget*	m_ew_locations;

		Internal()
			: m_tab_bar(nullptr)
			, m_view(nullptr)
			, m_ew_items(nullptr)
			, m_ew_locations(nullptr)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapWidget::MapWidget(EditorInterface& editor_interface, QWidget* parent)
		: QWidget(parent)
		, m_internal(std::make_unique<Internal>())
	{
		// Widget.
		setWindowTitle("Map View");
		setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
		setMouseTracking(true);
		auto layout = new QVBoxLayout();
		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);

		m_internal->m_tab_bar = new QTabBar();
		connect(m_internal->m_tab_bar, &QTabBar::currentChanged, this, &MapWidget::slot_tab_changed);
		layout->addWidget(m_internal->m_tab_bar);

		QApplication::instance()->installEventFilter(this);

		// View.
		m_internal->m_view = new MapView(nullptr);
		m_internal->m_view->setContextMenuPolicy(Qt::CustomContextMenu);
		m_internal->m_view->viewport()->setMouseTracking(true);
		connect(m_internal->m_view, &MapView::customContextMenuRequested, this, &MapWidget::slot_context_menu);
		layout->addWidget(m_internal->m_view);
		
		// Scenes.
		auto create_scene = [this, &editor_interface] (QString tab_name, QString tab_icon, MapSceneType type)
		{
			auto scene = new MapScene(editor_interface, type, this);
			connect(scene, &MapScene::selectionChanged, this, &MapWidget::slot_selection_changed);
			m_internal->m_scenes << scene;
			m_internal->m_tab_bar->addTab(QIcon(tab_icon), tab_name);
		};

		create_scene("Light World", ":/UI/ButtonLW", MapSceneType::LightWorld);
		create_scene("Dark World", ":/UI/ButtonDW", MapSceneType::DarkWorld);

		// Entity widgets.
		auto setup_entity_widget = [this, &editor_interface] (MapEntityWidget*& widget, Qt::Alignment alignment, QString filename)
		{
			widget = new MapEntityWidget(editor_interface, alignment, this);
			connect(widget, &MapEntityWidget::signal_clicked, this, &MapWidget::slot_close_entity_widgets);

			auto load_result = widget->load(filename);
			report_result(load_result, this, "Entity Widget Layout");
		};

		setup_entity_widget(m_internal->m_ew_items, Qt::AlignRight, get_absolute_path(editor_interface.get_settings().get().m_general_layout_instance_items));
		setup_entity_widget(m_internal->m_ew_locations, Qt::AlignLeft, get_absolute_path(editor_interface.get_settings().get().m_general_layout_instance_locations));

		// Signals.
		connect(&editor_interface.get_settings(), &Settings::signal_changed, this, &MapWidget::slot_settings_changed);

		// Initial state.
		m_internal->m_view->set_scene(*m_internal->m_scenes[0], true);
	}

	MapWidget::~MapWidget()
	{
	}



	//================================================================================
	// Schema
	//================================================================================

	void MapWidget::set_schema(SchemaPtr schema)
	{
		for (auto scene : m_internal->m_scenes)
		{
			scene->set_schema(schema);
		}

		m_internal->m_schema = schema;
	}

	void MapWidget::clear_schema()
	{
		for (auto scene : m_internal->m_scenes)
		{
			scene->clear_schema();
		}

		m_internal->m_schema = nullptr;
	}



	//================================================================================
	// Instance
	//================================================================================

	void MapWidget::set_instance(InstancePtr instance)
	{
		for (auto scene : m_internal->m_scenes)
		{
			scene->set_instance(instance);
		}

		m_internal->m_view->set_instance(instance);

		m_internal->m_instance = instance;
	}

	void MapWidget::clear_instance()
	{
		for (auto scene : m_internal->m_scenes)
		{
			scene->clear_instance();
		}

		m_internal->m_view->clear_instance();

		m_internal->m_instance = nullptr;
	}



	//================================================================================
	// Selection
	//================================================================================

	void MapWidget::select_schema_item(SchemaItemPtr schema_item)
	{
		for (auto scene : m_internal->m_scenes)
		{
			auto items = scene->items();
			auto it = std::find_if(items.begin(), items.end(), [scene, schema_item] (QGraphicsItem* scene_item)
			{
				return (scene->get_item_type(*scene_item) == MapSceneItemType::SchemaItem &&
						static_cast<MapSceneItemSchemaItem*>(scene_item)->get_schema_item() == schema_item);
			});

			if (it != items.end())
			{
				scene->clearSelection();
				activate_scene(*scene);
				(*it)->setSelected(true);
			}
		}
	}



	//================================================================================
	// Qt Events
	//================================================================================

	bool MapWidget::eventFilter(QObject* /*object*/, QEvent* event)
	{
		auto is_ancestor_of = [this] (QWidget* parent, QWidget* widget)
		{
			while (widget)
			{
				if (widget == parent)
				{
					return true;
				}
				widget = widget->parentWidget();
			}
			return false;
		};

		auto should_close = [&is_ancestor_of, event] (QWidget* widget)
		{
			if (event->type() == QEvent::MouseButtonPress && widget != nullptr && widget->isVisible())
			{
				auto child = QApplication::widgetAt(static_cast<QMouseEvent*>(event)->globalPos());
				return !is_ancestor_of(widget, child);
			}
			return false;
		};

		if (should_close(m_internal->m_ew_items) && should_close(m_internal->m_ew_locations))
		{
			m_internal->m_ew_items->hide();
			m_internal->m_ew_locations->hide();
		}

		return false;
	}



	//================================================================================
	// UI Slots
	//================================================================================

	void MapWidget::slot_tab_changed(int index)
	{
		activate_scene(*m_internal->m_scenes[index]);
	}

	void MapWidget::slot_context_menu()
	{
		auto view = m_internal->m_view;
		auto scene = view->get_scene();

		// Schema items.
		if (m_internal->m_schema != nullptr)
		{
			bool has_selected_item = !scene->selectedItems().isEmpty();

			QMenu menu(this);
			menu.addAction("Add Item", this, &MapWidget::slot_add_schema_item)->setProperty("Position", QCursor::pos());
			menu.addAction("Remove Items", this, &MapWidget::slot_remove_schema_items)->setEnabled(has_selected_item);
			menu.exec(QCursor::pos());
		}

		// Instance items.
		if (m_internal->m_instance != nullptr)
		{
			auto pos_global = QCursor::pos();
			auto pos_view = view->viewport()->mapFromGlobal(pos_global);
			auto pos_scene = view->mapToScene(pos_view);

			auto item = scene->itemAt(pos_scene, view->transform());
			if (item != nullptr && scene->get_item_type(*item) == MapSceneItemType::InstanceItem)
			{
				auto instance_item = static_cast<MapSceneItemInstanceItem*>(item)->get_instance_item();
				if (!instance_item->get().m_cleared)
				{
					if (!(QApplication::keyboardModifiers() & Qt::ShiftModifier))
					{
						m_internal->m_ew_items->show();
						m_internal->m_ew_items->set_item(instance_item);
						int items_x = QCursor::pos().x() - m_internal->m_ew_items->width();
						int items_y = QCursor::pos().y() - m_internal->m_ew_items->height() / 2;
						m_internal->m_ew_items->move(items_x, items_y);

						m_internal->m_ew_locations->show();
						m_internal->m_ew_locations->set_item(instance_item);
						int locations_x = QCursor::pos().x() + 5;
						int locations_y = QCursor::pos().y() - m_internal->m_ew_locations->height() / 2;
						m_internal->m_ew_locations->move(locations_x, locations_y);
					}
					else
					{
						auto data = instance_item->get();
						data.m_items.clear();
						data.m_location = nullptr;
						data.m_location_entrance = nullptr;
						instance_item->set(data);
					}
				}
			}
		}
	}

	void MapWidget::slot_add_schema_item()
	{
		auto item = m_internal->m_schema->items().add();
		auto scene = static_cast<MapScene*>(m_internal->m_view->scene());
		auto pos = sender()->property("Position").toPoint();

		auto data = item->get();
		data.m_map = EnumReflection<MapSceneType,MapSceneTypeInfo>::info(scene->get_type()).m_schema_item_map_type;
		data.m_position = m_internal->m_view->mapToScene(m_internal->m_view->mapFromGlobal(pos));
		item->set(data);
	}

	void MapWidget::slot_remove_schema_items()
	{
		for (auto scene_item : m_internal->m_view->scene()->selectedItems())
		{
			if (m_internal->m_view->get_scene()->get_item_type(*scene_item) == MapSceneItemType::SchemaItem)
			{
				auto schema_item = static_cast<MapSceneItemSchemaItem*>(scene_item)->get_schema_item();
				m_internal->m_schema->items().remove(schema_item);
			}
		}
	}

	void MapWidget::slot_close_entity_widgets()
	{
		m_internal->m_ew_items->hide();
		m_internal->m_ew_locations->hide();
	}



	//================================================================================
	// Scene Slots
	//================================================================================

	void MapWidget::slot_selection_changed()
	{
		emit signal_selection_changed(m_internal->m_view->scene()->selectedItems().toVector());
	}



	//================================================================================
	// Settings Slots
	//================================================================================

	void MapWidget::slot_settings_changed(const SettingsDiff& diff)
	{
		if (diff.has_change(&SettingsData::m_general_layout_instance_items))
		{
			auto load_result = m_internal->m_ew_items->load(diff.m_new.m_general_layout_instance_items);
			report_result(load_result, this, "Entity Widget Layout");
		}

		if (diff.has_change(&SettingsData::m_general_layout_instance_locations))
		{
			auto load_result = m_internal->m_ew_locations->load(diff.m_new.m_general_layout_instance_locations);
			report_result(load_result, this, "Entity Widget Layout");
		}
	}



	//================================================================================
	// Helpers
	//================================================================================

	void MapWidget::activate_scene(MapScene& scene)
	{
		int index = m_internal->m_scenes.indexOf(&scene);
		Q_ASSERT(index != -1);

		m_internal->m_tab_bar->setCurrentIndex(index);
		m_internal->m_view->set_scene(*m_internal->m_scenes[index], false);
		slot_selection_changed();
	}
}