// Project includes
#include "UI/MapWidget/MapEntityWidget.h"
#include "UI/EntityWidget/EntityWidgetItem.h"
#include "Data/Database/ItemDatabase.h"
#include "Data/Database/LocationDatabase.h"
#include "Data/Instance/InstanceData.h"
#include "Data/DataModel.h"
#include "EditorInterface.h"

// Qt includes
#include <QMenu>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct MapEntityWidget::Internal
	{
		const ItemDatabase&		m_item_db;
		const LocationDatabase&	m_location_db;
		InstanceItemPtr			m_instance_item;

		Internal(const ItemDatabase& item_db, const LocationDatabase& location_db)
			: m_item_db(item_db)
			, m_location_db(location_db)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	MapEntityWidget::MapEntityWidget(const EditorInterface& editor_interface, Qt::Alignment alignment, QWidget* parent)
		: EntityWidget(editor_interface.get_data_model().get_entity_db(), alignment, parent)
		, m_internal(std::make_unique<Internal>(editor_interface.get_data_model().get_item_db(), editor_interface.get_data_model().get_location_db()))
	{
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
		setAttribute(Qt::WA_TranslucentBackground);
	}

	MapEntityWidget::~MapEntityWidget()
	{
	}



	//================================================================================
	// Item
	//================================================================================

	void MapEntityWidget::set_item(InstanceItemPtr instance_item)
	{
		m_internal->m_instance_item = instance_item;
		sync();
	}



	//================================================================================
	// Events
	//================================================================================

	void MapEntityWidget::on_clicked(EntityWidgetItem* entity_item, Qt::MouseButton button)
	{
		auto entity = entity_item->get_entity();
		auto item = m_internal->m_item_db.get_item(entity);
		auto location = m_internal->m_location_db.get_location(entity);

		auto instance_data = m_internal->m_instance_item->get();

		if (item != nullptr && button == Qt::LeftButton)
		{
			if (!instance_data.m_items.contains(item))
			{
				instance_data.m_items.push_back(item);
			}
			else
			{
				instance_data.m_items.removeOne(item);
			}
		}

		if (location != nullptr && button == Qt::LeftButton)
		{
			instance_data.m_location = location;
			instance_data.m_location_entrance = nullptr;

			if (!location->m_entrances.isEmpty())
			{
				QMenu menu(this);

				for (auto entrance : location->m_entrances)
				{
					menu.addAction(entrance->m_image, entrance->m_display_name, [&instance_data, entrance] ()
					{
						instance_data.m_location_entrance = entrance;
					});
				}

				menu.addSeparator();
				menu.addAction("None");

				menu.exec(QCursor::pos());
			}
		}

		m_internal->m_instance_item->set(instance_data);

		sync();
	}



	//================================================================================
	// Helpers
	//================================================================================

	void MapEntityWidget::sync()
	{
		auto& instance_data = m_internal->m_instance_item->get();
		
		auto entity_items = get_items();
		for (auto entity_item : entity_items)
		{
			auto entity_index = entity_item->get_entity_index();
			auto entity_item_data = entity_item->get_data(entity_index);
			
			auto item = m_internal->m_item_db.get_item(entity_item_data.m_entity);
			auto location = m_internal->m_location_db.get_location(entity_item_data.m_entity);

			if (item != nullptr)
			{
				entity_item_data.m_alpha = (instance_data.m_items.contains(item) ? 0.3f : 1.0f);
			}

			if (location != nullptr)
			{
				entity_item_data.m_alpha = (location == instance_data.m_location ? 0.3f : 1.0f);
			}

			entity_item->set_data(entity_index, entity_item_data);
		}
	}
}