// Project includes
#include "UI/ProgressItemWidget/ProgressItemWidget.h"
#include "UI/EntityWidget/EntityWidgetItem.h"
#include "Data/Database/ItemDatabase.h"
#include "Data/Instance/Instance.h"
#include "Data/DataModel.h"
#include "Data/Settings.h"
#include "Utility/File.h"
#include "Utility/JSON.h"
#include "EditorInterface.h"

// Qt includes
#include <QJsonArray>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct ProgressItemWidget::Internal
	{
		const ItemDatabase& m_item_db;
		InstancePtr			m_instance;
		
		Internal(const ItemDatabase& item_db)
			: m_item_db(item_db)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	ProgressItemWidget::ProgressItemWidget(const EditorInterface& editor_interface, QWidget* parent)
		: EntityWidget(editor_interface.get_data_model().get_entity_db(), Qt::AlignLeft, parent)
		, m_internal(std::make_unique<Internal>(editor_interface.get_data_model().get_item_db()))
	{
		// Widget.
		setWindowTitle("Progress Items");

		// Entity layout.
		auto load_result = load(get_absolute_path(editor_interface.get_settings().get().m_general_layout_progress_items));
		report_result(load_result, this, "Progress Items Layout");

		// Signals.
		connect(&editor_interface.get_settings(), &Settings::signal_changed, this, &ProgressItemWidget::slot_settings_changed);

		// Initial state.
		clear_instance();
	}

	ProgressItemWidget::~ProgressItemWidget()
	{
	}



	//================================================================================
	// Instance
	//================================================================================

	void ProgressItemWidget::set_instance(InstancePtr instance)
	{
		clear_instance();

		m_internal->m_instance = instance;

		sync_from_progress();
	}

	void ProgressItemWidget::clear_instance()
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

	void ProgressItemWidget::on_clicked(EntityWidgetItem* entity_item, Qt::MouseButton button)
	{
		if (m_internal->m_instance == nullptr)
		{
			return;
		}

		int index = entity_item->get_entity_index();
			
		switch (button)
		{
		case Qt::LeftButton: ++index; break;
		case Qt::RightButton: --index; break;
		default: return;
		}

		entity_item->set_entity_index(index);

		sync_to_progress(entity_item);
	}



	//================================================================================
	// Creators
	//================================================================================

	Result ProgressItemWidget::create_item(const QJsonValue& json, EntityWidgetItem*& item)
	{
		Result result;
		QVector<EntityWidgetItemData> data_list;

		QVector<QJsonValue> json_values;

		if (json.isArray())
		{
			for (auto jval : json.toArray())
			{
				json_values << jval;
			}
		}
		else
		{
			json_values << json;
		}

		for (auto json_value : json_values)
		{
			EntityWidgetItemData data;
			auto data_result = create_data(json_value, data);
			result << data_result;
			if (!data_result)
			{
				continue;
			}

			if (data.m_attributes.contains("ctr"))
			{
				int num = data.m_attributes["ctr"].toInt();

				for (int i = 0; i < num; ++i)
				{
					data.m_text = QString("%1/%2").arg(i+1).arg(num);
					data_list << data;
				}

				continue;
			}

			data_list << data;
		}

		if (!data_list.isEmpty())
		{
			auto disabled_data = data_list[0];
			disabled_data.m_attributes["disabled"] = true;
			disabled_data.m_alpha = 0.3f;
			disabled_data.m_text.clear();
			data_list.push_front(disabled_data);

			item = new EntityWidgetItem(data_list);
		}

		return result;
	}



	//================================================================================
	// Settings Slots
	//================================================================================

	void ProgressItemWidget::slot_settings_changed(const SettingsDiff& diff)
	{
		if (diff.has_change(&SettingsData::m_general_layout_progress_items))
		{
			auto load_result = load(diff.m_new.m_general_layout_progress_items);
			report_result(load_result, this, "Progress Items Layout");
		}
	}



	//================================================================================
	// Helpers
	//================================================================================

	void ProgressItemWidget::sync_from_progress()
	{
		auto items = get_items();
			
		for (auto progress_item : m_internal->m_instance->progress_items().get())
		{
			auto it = std::find_if(items.begin(), items.end(), [progress_item] (EntityWidgetItem* item)
			{
				return item->get_entities().contains(progress_item->get().m_item->m_entity);
			});

			if (it != items.end())
			{
				(*it)->set_entity_index((*it)->get_entity_index() + progress_item->get().m_num);
			}
		}
	}

	void ProgressItemWidget::sync_to_progress(EntityWidgetItem* entity_item)
	{
		// Clear old data.
		auto entities = entity_item->get_entities();
		for (auto entity : entities)
		{
			auto item = m_internal->m_instance->progress_items().find(entity);
			if (item != nullptr)
			{
				m_internal->m_instance->progress_items().remove(item);
			}
		}

		// Set new data.
		int entity_index = entity_item->get_entity_index();
		for (int i = 0; i <= entity_index; ++i)
		{
			auto& entity_data = entity_item->get_data(i);
			if (entity_data.m_attributes.contains("disabled"))
			{
				continue;
			}

			auto progress_item = m_internal->m_instance->progress_items().find(entity_data.m_entity);
			if (progress_item == nullptr)
			{
				auto item = m_internal->m_item_db.get_item(entity_data.m_entity);
				if (item != nullptr)
				{
					m_internal->m_instance->progress_items().add(item);
				}
			}
			else
			{
				auto item_data = progress_item->get();
				item_data.m_num++;
				progress_item->set(item_data);
			}
		}
	}
}