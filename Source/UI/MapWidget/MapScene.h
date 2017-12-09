#ifndef MAP_SCENE_H
#define MAP_SCENE_H

// Project includes
#include "Data/Schema/SchemaData.h"
#include "EditorTypeInfo.h"
#include "Utility/EnumReflection.h"

// Qt includes
#include <QGraphicsScene>

// Stdlib includes
#include <memory>

// Forward declarations
namespace LTTPMapTracker
{
	class EditorInterface;
	struct SettingsDiff;
}


namespace LTTPMapTracker
{
	// Types
	//--------------------------------------------------------------------------------

	enum class MapSceneType
	{
		LightWorld,
		DarkWorld
	};

	struct MapSceneTypeInfo
	{
		MapSceneType	m_type;
		SchemaItemMap	m_schema_item_map_type;
		QString			m_type_name;
		QString			m_image_path;
	};

	enum class MapSceneItemType
	{
		Background,
		SchemaItem,
		InstanceItem,
		Connection
	};


	// Map Scene
	//--------------------------------------------------------------------------------

	class MapScene : public QGraphicsScene
	{
		Q_OBJECT

	public:
		// Construction & Destruction
								MapScene								(EditorInterface& editor_interface, MapSceneType type, QObject* parent = nullptr);
								~MapScene								();

		// Properties
		MapSceneType			get_type								() const;
		MapSceneItemType		get_item_type							(const QGraphicsItem& item) const;

		// Schema
		void					set_schema								(SchemaPtr schema);
		void					clear_schema							();

		// Instance
		void					set_instance							(InstancePtr instance);
		void					clear_instance							();

	private slots:
		// Settings Slots
		void					slot_settings_changed					(const SettingsDiff& diff);

		// Schema Slots
		void					slot_schema_item_added					(int index);
		void					slot_schema_item_to_be_removed			(int index);
		void					slot_schema_item_modified				(int index);
		void					slot_instance_connection_added			(int index);
		void					slot_instance_connection_to_be_removed	(int index);

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

DECLARE_ENUM_REFLECTION_INFO(LTTPMapTracker::MapSceneType, LTTPMapTracker::MapSceneTypeInfo);

#endif