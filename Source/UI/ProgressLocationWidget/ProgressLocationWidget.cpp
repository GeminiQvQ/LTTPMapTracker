// Project includes
#include "UI/ProgressLocationWidget/ProgressLocationWidget.h"
#include "UI/EntityWidget/EntityWidgetItem.h"
#include "Data/Database/EntityDatabase.h"
#include "Data/Database/LocationDatabase.h"
#include "Data/Instance/Instance.h"
#include "Data/DataModel.h"
#include "Data/Settings.h"
#include "Utility/File.h"
#include "EditorInterface.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct ProgressLocationWidget::Internal
	{
		const EntityDatabase&	m_entity_db;
		const LocationDatabase& m_location_db;
		InstancePtr				m_instance;

		Internal(const EntityDatabase& entity_db, const LocationDatabase& location_db)
			: m_entity_db(entity_db)
			, m_location_db(location_db)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	ProgressLocationWidget::ProgressLocationWidget(const EditorInterface& editor_interface, QWidget* parent)
		: EntityWidget(editor_interface.get_data_model().get_entity_db(), Qt::AlignLeft, parent)
		, m_internal(std::make_unique<Internal>(editor_interface.get_data_model().get_entity_db(), editor_interface.get_data_model().get_location_db()))
	{
		// Widget.
		setWindowTitle("Progress Locations");

		// Entity layout.
		auto load_result = load(get_absolute_path(editor_interface.get_settings().get().m_general_layout_progress_locations));
		report_result(load_result, this, "Progress Locations Layout");

		// Signals.
		connect(&editor_interface.get_settings(), &Settings::signal_changed, this, &ProgressLocationWidget::slot_settings_changed);

		// Initial state.
		clear_instance();
	}

	ProgressLocationWidget::~ProgressLocationWidget()
	{
	}



	//================================================================================
	// Loading
	//================================================================================

	Result ProgressLocationWidget::load(QString filename)
	{
		// Clear old data.
		clear();

		// Load data.
		QJsonObject json;
		
		auto load_result = json_load(json, filename);
		if (!load_result)
		{
			return load_result;
		}

		// Deserialize.
		int version = 0;
		if (json_read(json, "Version", version, 0).get_type() != ResultType::Ok)
		{
			return Result(false, "Unable to read version.");
		}

		Result result;

		// Settings.
		QJsonValue jval_settings;
		json_read(json, "Settings", jval_settings);
		auto json_settings = jval_settings.toObject();

		bool include_key_chests = false;
		json_read(json_settings, "IncludeKeyChests", include_key_chests, false);
		bool include_etc_chests = false;
		json_read(json_settings, "IncludeEtcChests", include_etc_chests, false);

		// Entity data.
		QJsonValue jval_entity_data;
		result << json_read(json, "EntityData", jval_entity_data);
		auto json_entity_data = jval_entity_data.toObject();

		QHash<QString, EntityCPtr> entity_data;

		auto process_entity_data = [this, &entity_data, &json_entity_data, &result] (QString name)
		{
			if (json_entity_data.contains(name))
			{
				auto entity_name = json_entity_data[name].toString();
				auto entity = m_internal->m_entity_db.get_entity(entity_name);
				if (entity != nullptr)
				{
					entity_data.insert(name, entity);
				}
				else
				{
					result << Result(ResultType::Warning, "Entity not found: " + entity_name);
				}
			}
		};

		process_entity_data("Blank");
		process_entity_data("DTUnknown");
		process_entity_data("DTCrystal");
		process_entity_data("DTCrystalRed");
		process_entity_data("DTPendant");
		process_entity_data("DTPendantGreen");
		process_entity_data("Chest");
		process_entity_data("ChestOpen");
		process_entity_data("Key");
		process_entity_data("BigKey");

		// Location data.
		QJsonValue jval_location_data_list;
		result << json_read(json, "LocationData", jval_location_data_list);
		for (auto jval_location_data : jval_location_data_list.toArray())
		{
			auto json_location_data = jval_location_data.toObject();

			QString name;
			result << json_read(json_location_data, "Name", name, QString());

			struct LocationData
			{
				LocationCPtr m_location;
				QStringList  m_components;
			};
			QVector<LocationData> locations;

			QJsonValue jval_locations;
			result << json_read(json_location_data, "Locations", jval_locations);
			for (auto jval_location : jval_locations.toArray())
			{
				auto json_location = jval_location.toArray();
				if (json_location.size() != 2)
				{
					continue;
				}

				auto location_name = json_location[0].toString();
				auto location = m_internal->m_location_db.get_location(location_name);
				if (location == nullptr)
				{
					result << Result(ResultType::Warning, "Location not found: " + location_name);
					continue;
				}

				QStringList components;
				components << "Main";
				for (auto jval_component : json_location[1].toArray())
				{
					components << jval_component.toString();
				}

				locations.push_back({location, components});
			}

			// Create items.
			for (int location_index = 0; location_index < locations.size(); ++location_index)
			{
				auto& location_data = locations[location_index];
				auto location = location_data.m_location;

				for (auto component : location_data.m_components)
				{
					QVector<EntityWidgetItemData> data_list;

					EntityWidgetItemData data;
					data.m_attributes.insert("Component", component);
					data.m_attributes.insert("Location", location->m_entity->m_type_name);

					if (component == "Main")
					{
						data.m_entity = location->m_entity;
						data.m_alpha = 0.3f;
						data_list << data;
						data.m_alpha = 1.0f;
						data.m_attributes.insert("Cleared", true);
						data_list << data;
					}

					if (component == "DungeonType")
					{
						auto process_type = [&data_list, &data, &entity_data] (QString name, float alpha = 1.0f)
						{
							auto it = entity_data.find(name);
							if (it != entity_data.end())
							{
								data.m_attributes.insert("Type", name);
								data.m_entity = *it;
								data.m_alpha = alpha;
								data_list << data;
							}
						};

						process_type("DTUnknown", 0.3f);
						process_type("DTCrystal");
						process_type("DTCrystalRed");
						process_type("DTPendantGreen");
						process_type("DTPendant");

						if (data_list.size() != 5)
						{
							data_list.clear();
						}
					}

					if (component == "Chest")
					{
						auto open_chest_it = entity_data.find("ChestOpen");

						auto it = entity_data.find("Chest");
						if (it != entity_data.end())
						{
							int num_chests = location->m_treasure;
							num_chests -= (!include_key_chests ? location->m_treasure_keys : 0);
							num_chests -= (!include_etc_chests ? location->m_treasure_etc : 0);
							
							data.m_entity = *it;
							data.m_alpha = 0.3f;
							data.m_text = QString("0/%2").arg(num_chests);
							data_list << data;
							data.m_alpha = 1.0f;

							for (int i = 1; i < num_chests; ++i)
							{
								data.m_text = QString("%1/%2").arg(i).arg(num_chests);
								data_list << data;
							}

							data.m_entity = (open_chest_it != entity_data.end() ? *open_chest_it : *it);
							data.m_text = QString("%1/%2").arg(num_chests).arg(num_chests);
							data_list << data;
						}
					}

					if (component == "KeyCurrent")
					{
						auto it = entity_data.find("Key");
						if (it != entity_data.end())
						{
							data.m_entity = *it;
							data.m_alpha = 0.3f;
							data_list << data;
							data.m_alpha = 1.0f;

							for (int i = 1; i <= location->m_treasure_keys; ++i)
							{
								data.m_text = QString("%1").arg(i);
								data_list << data;
							}
						}
					}

					if (component == "KeyTotal")
					{
						auto it = entity_data.find("Key");
						if (it != entity_data.end())
						{
							data.m_entity = *it;
							data.m_alpha = 0.3f;
							data.m_text = QString("0/%2").arg(location->m_treasure_keys);
							data_list << data;
							data.m_alpha = 1.0f;

							for (int i = 1; i <= location->m_treasure_keys; ++i)
							{
								data.m_text = QString("%1/%2").arg(i).arg(location->m_treasure_keys);
								data_list << data;
							}
						}
					}

					if (component == "BigKey")
					{
						auto it = entity_data.find("BigKey");
						if (it != entity_data.end())
						{
							data.m_entity = *it;
							data.m_alpha = 0.3f;
							data_list << data;
							data.m_alpha = 1.0f;
							data_list << data;
						}
					}

					if (component == "Blank")
					{
						auto it = entity_data.find("Blank");
						if (it != entity_data.end())
						{
							data.m_entity = *it;
							data.m_static = true;
							data_list << data;
						}
					}
					
					if (!data_list.isEmpty())
					{
						add_item(*new EntityWidgetItem(data_list), name, location_index);
					}
				}
			}
		}

		fit_layouts();

		return result;
	}



	//================================================================================
	// Instance
	//================================================================================

	void ProgressLocationWidget::set_instance(InstancePtr instance)
	{
		clear_instance();

		m_internal->m_instance = instance;

		sync_from_progress();
	}

	void ProgressLocationWidget::clear_instance()
	{
		for (auto item : get_items())
		{
			item->set_entity_index(0);
		}

		m_internal->m_instance = nullptr;
	}



	//================================================================================
	// Events
	//================================================================================

	void ProgressLocationWidget::on_clicked(EntityWidgetItem* entity_item, Qt::MouseButton button)
	{
		// Update entity index.
		int index = entity_item->get_entity_index();
			
		switch (button)
		{
		case Qt::LeftButton: ++index; break;
		case Qt::RightButton: --index; break;
		default: return;
		}

		entity_item->set_entity_index(index);

		// Sync location data.
		sync_to_progress(entity_item);
		sync_from_progress();
	}



	//================================================================================
	// Settings Slots
	//================================================================================

	void ProgressLocationWidget::slot_settings_changed(const SettingsDiff& diff)
	{
		if (diff.has_change(&SettingsData::m_general_layout_progress_locations))
		{
			auto load_result = load(diff.m_new.m_general_layout_progress_locations);
			report_result(load_result, this, "Progress Locations Layout");
		}
	}



	//================================================================================
	// Helpers
	//================================================================================

	void ProgressLocationWidget::sync_from_progress()
	{
		for (auto item : get_items())
		{
			auto& entity_data = item->get_data(0);

			auto location_name = entity_data.m_attributes["Location"].toString();
			auto location = m_internal->m_location_db.get_location(location_name);
			auto progress_location = m_internal->m_instance->progress_locations().find(location->m_entity);
			auto& progress_location_data = progress_location->get();

			auto component = entity_data.m_attributes["Component"].toString();

			if (component == "Main")
			{
				item->set_entity_index(progress_location_data.m_cleared ? 1 : 0);
			}

			if (component == "DungeonType")
			{
				int index = 0;
				if (progress_location_data.m_is_crystal) index = 1;
				if (progress_location_data.m_is_crystal_red) index = 2;
				if (progress_location_data.m_is_pendant_green) index = 3;
				if (progress_location_data.m_is_pendant) index = 4;
				item->set_entity_index(index);
				
				auto data = item->get_data(index);
				data.m_alpha = (progress_location->get().m_cleared ? 1.0f : 0.3f);
				item->set_data(index, data);
			}

			if (component == "Chest")
			{
				item->set_entity_index(qMin(progress_location_data.m_num_items, item->get_entities().size() - 1));
			}

			if (component == "KeyCurrent")
			{
				item->set_entity_index(qMin(progress_location_data.m_num_keys_current, item->get_entities().size() - 1));
			}

			if (component == "KeyTotal")
			{
				item->set_entity_index(qMin(progress_location_data.m_num_keys_total, item->get_entities().size() - 1));
			}

			if (component == "BigKey")
			{
				item->set_entity_index(progress_location_data.m_has_big_key ? 1 : 0);
			}

			if (component == "Map")
			{
				item->set_entity_index(progress_location_data.m_has_map ? 1 : 0);
			}

			if (component == "Compass")
			{
				item->set_entity_index(progress_location_data.m_has_compass ? 1 : 0);
			}
		}
	}

	void ProgressLocationWidget::sync_to_progress(EntityWidgetItem* entity_item)
	{
		auto entity_index = entity_item->get_entity_index();
		auto& entity_data = entity_item->get_data(entity_index);

		auto location_name = entity_data.m_attributes["Location"].toString();
		auto location = m_internal->m_location_db.get_location(location_name);
		auto progress_location = m_internal->m_instance->progress_locations().find(location->m_entity);
		auto progress_location_data = progress_location->get();
		
		auto component = entity_data.m_attributes["Component"].toString();

		if (component == "Main")
		{
			progress_location_data.m_cleared = entity_data.m_attributes["Cleared"].toBool();
		}

		if (component == "DungeonType")
		{
			progress_location_data.m_is_crystal = (entity_data.m_attributes["Type"].toString() == "DTCrystal");
			progress_location_data.m_is_crystal_red = (entity_data.m_attributes["Type"].toString() == "DTCrystalRed");
			progress_location_data.m_is_pendant = (entity_data.m_attributes["Type"].toString() == "DTPendant");
			progress_location_data.m_is_pendant_green = (entity_data.m_attributes["Type"].toString() == "DTPendantGreen");
		}

		if (component == "Chest")
		{
			progress_location_data.m_num_items = entity_index;
		}

		if (component == "KeyCurrent")
		{
			progress_location_data.m_num_keys_current = entity_index;
		}

		if (component == "KeyTotal")
		{
			progress_location_data.m_num_keys_total = entity_index;
		}

		if (component == "BigKey")
		{
			progress_location_data.m_has_big_key = (entity_index == 1 ? true : false);
		}

		if (component == "Map")
		{
			progress_location_data.m_has_map = (entity_index == 1 ? true : false);
		}

		if (component == "Compass")
		{
			progress_location_data.m_has_compass = (entity_index == 1 ? true : false);
		}

		progress_location->set(progress_location_data);
	}
}