// Project includes
#include "UI/MapWidget/MapScene.h"
#include "UI/MapWidget/Items/MapSceneItemBackground.h"
#include "UI/MapWidget/Items/MapSceneItemConnectionItem.h"
#include "UI/MapWidget/Items/MapSceneItemInstanceItem.h"
#include "UI/MapWidget/Items/MapSceneItemSchemaItem.h"
#include "Data/Instance/Instance.h"
#include "Data/Instance/InstanceData.h"
#include "Data/Schema/Schema.h"
#include "Data/Schema/SchemaData.h"
#include "Data/Settings.h"
#include "EditorInterface.h"


namespace Utility
{
	//================================================================================
	// Enum Reflection
	//================================================================================

	DEFINE_ENUM_REFLECTION_INFO(LTTPMapTracker::MapSceneType, LTTPMapTracker::MapSceneTypeInfo)
	{
		//	Type										Schema Item Map Type						Type Name	Image Name
		{	LTTPMapTracker::MapSceneType::LightWorld,	LTTPMapTracker::SchemaItemMap::LightWorld,	"LW",		":/UI/MapLW"	},
		{	LTTPMapTracker::MapSceneType::DarkWorld,	LTTPMapTracker::SchemaItemMap::DarkWorld,	"DW",		":/UI/MapDW"	}
	};



	//================================================================================
	// Lifetime
	//================================================================================

	void delete_later(QGraphicsItem* item)
	{
		class Deleter : public QObject
		{
		public:
			Deleter(QGraphicsItem* item) : m_item(item) { }
			~Deleter() { delete m_item; }

		private:
			QGraphicsItem* m_item;
		};

		auto deleter = new Deleter(item);
		deleter->deleteLater();
	}
}


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct MapScene::Internal
	{
		using SceneItemTypeMap = QMap<const QGraphicsItem*, MapSceneItemType>;

		EditorInterface&		m_editor_interface;
		MapSceneType			m_type;
		MapSceneItemBackground* m_bg_item;
		SceneItemTypeMap		m_item_types;

		SchemaPtr				m_schema;
		InstancePtr				m_instance;

		Internal(EditorInterface& editor_interface, MapSceneType type)
			: m_editor_interface(editor_interface)
			, m_type(type)
			, m_bg_item()
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapScene::MapScene(EditorInterface& editor_interface, MapSceneType type, QObject* parent)
		: QGraphicsScene(parent)
		, m_internal(std::make_unique<Internal>(editor_interface, type))
	{
		// Signals.
		connect(&editor_interface.get_settings(), &Settings::signal_changed, this, &MapScene::slot_settings_changed);

		// Initial state.
		m_internal->m_bg_item = new MapSceneItemBackground(editor_interface, EnumReflection<MapSceneType, MapSceneTypeInfo>::info(type).m_image_path);
		m_internal->m_item_types.insert(m_internal->m_bg_item, MapSceneItemType::Background);
		addItem(m_internal->m_bg_item);
		setSceneRect(m_internal->m_bg_item->boundingRect().translated(m_internal->m_bg_item->pos()));
	}

	MapScene::~MapScene()
	{
	}



	//================================================================================
	// Properties
	//================================================================================

	MapSceneType MapScene::get_type() const
	{
		return m_internal->m_type;
	}

	MapSceneItemType MapScene::get_item_type(const QGraphicsItem& item) const
	{
		auto it = m_internal->m_item_types.find(&item);
		Q_ASSERT(it != m_internal->m_item_types.end());
		return *it;
	}



	//================================================================================
	// Schema
	//================================================================================

	void MapScene::set_schema(SchemaPtr schema)
	{
		clear_schema();

		for (auto schema_item : schema->items().get())
		{
			if (schema_item->get().m_map == EnumReflection<MapSceneType, MapSceneTypeInfo>::info(m_internal->m_type).m_schema_item_map_type)
			{
				auto scene_item = new MapSceneItemSchemaItem(m_internal->m_editor_interface, schema_item);
				m_internal->m_item_types.insert(scene_item, MapSceneItemType::SchemaItem);
				addItem(scene_item);
			}
		}

		connect(&schema->items(), &SchemaItems::signal_added, this, &MapScene::slot_schema_item_added);
		connect(&schema->items(), &SchemaItems::signal_to_be_removed, this, &MapScene::slot_schema_item_to_be_removed);
		connect(&schema->items(), &SchemaItems::signal_modified, this, &MapScene::slot_schema_item_modified);

		m_internal->m_schema = schema;
	}

	void MapScene::clear_schema()
	{
		if (m_internal->m_schema != nullptr)
		{
			m_internal->m_schema->items().disconnect(this);

			removeItem(m_internal->m_bg_item);
			clear();
			addItem(m_internal->m_bg_item);

			m_internal->m_schema = nullptr;
		}
	}



	//================================================================================
	// Instance
	//================================================================================

	void MapScene::set_instance(InstancePtr instance)
	{
		clear_instance();

		for (auto instance_item : instance->items())
		{
			auto schema_item = instance_item->get().m_schema_item;
			if (schema_item->get().m_map == EnumReflection<MapSceneType, MapSceneTypeInfo>::info(m_internal->m_type).m_schema_item_map_type)
			{
				auto scene_item = new MapSceneItemInstanceItem(m_internal->m_editor_interface, instance, instance_item);
				m_internal->m_item_types.insert(scene_item, MapSceneItemType::InstanceItem);
				addItem(scene_item);
			}
		}

		for (auto connection : instance->connections().get())
		{
			auto schema_item = connection->get().m_items[0]->get().m_schema_item;
			if (schema_item->get().m_map == EnumReflection<MapSceneType, MapSceneTypeInfo>::info(m_internal->m_type).m_schema_item_map_type)
			{
				auto scene_item = new MapSceneItemConnectionItem(m_internal->m_editor_interface, instance, connection);
				m_internal->m_item_types.insert(scene_item, MapSceneItemType::Connection);
				addItem(scene_item);
			}
		}

		connect(&instance->connections(), &InstanceConnections::signal_added, this, &MapScene::slot_instance_connection_added);
		connect(&instance->connections(), &InstanceConnections::signal_to_be_removed, this, &MapScene::slot_instance_connection_to_be_removed);

		m_internal->m_instance = instance;
	}

	void MapScene::clear_instance()
	{
		if (m_internal->m_instance != nullptr)
		{
			m_internal->m_instance->connections().disconnect(this);

			removeItem(m_internal->m_bg_item);
			clear();
			addItem(m_internal->m_bg_item);

			m_internal->m_instance = nullptr;
		}
	}



	//================================================================================
	// Settings Slots
	//================================================================================

	void MapScene::slot_settings_changed(const SettingsDiff& diff)
	{
		if (diff.has_change(&SettingsData::m_map_item_size) ||
			diff.has_change(&SettingsData::m_map_item_opacity_cleared) ||
			diff.has_change(&SettingsData::m_map_item_color_base) ||
			diff.has_change(&SettingsData::m_map_item_color_inaccessible) ||
			diff.has_change(&SettingsData::m_map_item_color_item_requirement) ||
			diff.has_change(&SettingsData::m_map_item_color_item_requirement_fulfilled) ||
			diff.has_change(&SettingsData::m_map_item_color_location) ||
			diff.has_change(&SettingsData::m_map_connection_thickness) ||
			diff.has_change(&SettingsData::m_map_connection_color))
		{
			if (m_internal->m_schema != nullptr)
			{
				set_schema(m_internal->m_schema);
			}

			if (m_internal->m_instance != nullptr)
			{
				set_instance(m_internal->m_instance);
			}
		}
	}



	//================================================================================
	// Schema Slots
	//================================================================================

	void MapScene::slot_schema_item_added(int index)
	{
		auto schema_item = m_internal->m_schema->items()[index];

		if (schema_item->get().m_map == EnumReflection<MapSceneType, MapSceneTypeInfo>::info(m_internal->m_type).m_schema_item_map_type)
		{
			auto scene_item = new MapSceneItemSchemaItem(m_internal->m_editor_interface, schema_item);
			m_internal->m_item_types.insert(scene_item, MapSceneItemType::SchemaItem);
			addItem(scene_item);
		}
	}

	void MapScene::slot_schema_item_to_be_removed(int index)
	{
		auto schema_item = m_internal->m_schema->items()[index];
		auto scene_items = this->items();

		auto it = std::find_if(scene_items.begin(), scene_items.end(), [this, schema_item] (QGraphicsItem* scene_item)
		{
			return (get_item_type(*scene_item) == MapSceneItemType::SchemaItem &&
					static_cast<MapSceneItemSchemaItem*>(scene_item)->get_schema_item() == schema_item);
		});

		if (it != scene_items.end())
		{
			removeItem(*it);
			delete_later(*it);
		}
	}

	void MapScene::slot_schema_item_modified(int index)
	{
		auto schema_item = m_internal->m_schema->items()[index];
		auto scene_items = this->items();

		auto it = std::find_if(scene_items.begin(), scene_items.end(), [this, schema_item] (QGraphicsItem* scene_item)
		{
			return (get_item_type(*scene_item) == MapSceneItemType::SchemaItem &&
					static_cast<MapSceneItemSchemaItem*>(scene_item)->get_schema_item() == schema_item);
		});

		if (it != scene_items.end() && schema_item->get().m_map != EnumReflection<MapSceneType, MapSceneTypeInfo>::info(m_internal->m_type).m_schema_item_map_type)
		{
			removeItem(*it);
			delete_later(*it);
		}

		if (it == scene_items.end() && schema_item->get().m_map == EnumReflection<MapSceneType, MapSceneTypeInfo>::info(m_internal->m_type).m_schema_item_map_type)
		{
			auto scene_item = new MapSceneItemSchemaItem(m_internal->m_editor_interface, schema_item);
			m_internal->m_item_types.insert(scene_item, MapSceneItemType::SchemaItem);
			addItem(scene_item);
		}
	}

	void MapScene::slot_instance_connection_added(int index)
	{
		auto connection = m_internal->m_instance->connections()[index];
		auto scene_items = this->items();

		auto map = connection->get().m_items[0]->get().m_schema_item->get().m_map;
		if (map == EnumReflection<MapSceneType, MapSceneTypeInfo>::info(m_internal->m_type).m_schema_item_map_type)
		{
			auto scene_item = new MapSceneItemConnectionItem(m_internal->m_editor_interface, m_internal->m_instance, connection);
			m_internal->m_item_types.insert(scene_item, MapSceneItemType::Connection);
			addItem(scene_item);
		}
	}

	void MapScene::slot_instance_connection_to_be_removed(int index)
	{
		auto connection = m_internal->m_instance->connections()[index];
		auto scene_items = this->items();

		auto it = std::find_if(scene_items.begin(), scene_items.end(), [this, connection] (QGraphicsItem* scene_item)
		{
			return (get_item_type(*scene_item) == MapSceneItemType::Connection &&
					static_cast<MapSceneItemConnectionItem*>(scene_item)->get_connection() == connection);
		});

		if (it != scene_items.end())
		{
			removeItem(*it);
			delete_later(*it);
		}
	}
}