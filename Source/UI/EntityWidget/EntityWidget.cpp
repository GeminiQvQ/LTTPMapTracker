// Project includes
#include "UI/EntityWidget/EntityWidget.h"
#include "UI/EntityWidget/EntityWidgetItem.h"
#include "Data/Database/EntityDatabase.h"
#include "Utility/JSON.h"

// Qt includes
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGroupBox>
#include <QJsonArray>
#include <QLayout>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct EntityWidget::Internal
	{
		struct Group
		{
			QGroupBox*						m_groupbox;
			QGraphicsView*					m_view;
			QGraphicsScene*					m_scene;
			QGraphicsLinearLayout*			m_scene_layout;
			QVector<QGraphicsLinearLayout*> m_row_layouts;
		};

		const EntityDatabase&		m_entity_db;
		QHash<QString, Group>		m_groups;
		QVector<EntityWidgetItem*>	m_items;
		Qt::Alignment				m_alignment;

		Internal(const EntityDatabase& entity_db, Qt::Alignment alignment)
			: m_entity_db(entity_db)
			, m_alignment(alignment)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	EntityWidget::EntityWidget(const EntityDatabase& entity_db, Qt::Alignment alignment, QWidget* parent)
		: QWidget(parent)
		, m_internal(std::make_unique<Internal>(entity_db, alignment))
	{
		// Widget.
		auto layout = new QVBoxLayout();
		layout->setAlignment(Qt::AlignTop);
		layout->setContentsMargins(3, 3, 3, 3);
		layout->setSpacing(0);
		setLayout(layout);
	}

	EntityWidget::~EntityWidget()
	{
	}



	//================================================================================
	// Loading
	//================================================================================

	Result EntityWidget::load(QString filename)
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
		
		QJsonValue jval_data;
		result << json_read(json, "Data", jval_data);
		for (auto jval_group : jval_data.toArray())
		{
			auto json_group = jval_group.toObject();

			QString group_name;
			result << json_read(json_group, "Name", group_name, QString());

			if (group_name.isEmpty())
			{
				result << ResultEntry(ResultType::Warning, "Empty group name.");
				continue;
			}

			QJsonValue jval_entities;
			result << json_read(json_group, "Entities", jval_entities);
			auto json_entities = jval_entities.toArray();
			
			for (int r = 0; r < json_entities.size(); ++r)
			{
				auto json_entity_list = json_entities[r].toArray();

				for (int c = 0; c < json_entity_list.size(); ++c)
				{
					EntityWidgetItem* item = nullptr;
					result << create_item(json_entity_list[c], item);
					
					if (item != nullptr)
					{
						add_item(*item, group_name, r);
					}
				}
			}
		}

		fit_layouts();

		return result;
	}



	//================================================================================
	// Items
	//================================================================================

	void EntityWidget::add_item(EntityWidgetItem& item, QString group, int row)
	{
		// Get and/or create group.
		auto group_it = m_internal->m_groups.find(group);
		if (group_it == m_internal->m_groups.end())
		{
			auto groupbox = new QGroupBox(group);
			auto groupbox_layout = new QVBoxLayout();
			groupbox_layout->setContentsMargins(0, 0, 0, 0);
			groupbox->setLayout(groupbox_layout);
			layout()->addWidget(groupbox);

			auto scene = new QGraphicsScene();
			auto view = new QGraphicsView();
			auto layout = new QGraphicsLinearLayout(Qt::Vertical);
			auto widget = new QGraphicsWidget();
			
			view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	

			widget->setLayout(layout);
			scene->addItem(widget);
			groupbox_layout->addWidget(view);
			view->setScene(scene);

			group_it = m_internal->m_groups.insert(group, { groupbox, view, scene, layout });
		}

		// Get and/or create layout.
		while (row >= group_it->m_row_layouts.size())
		{
			auto layout = new QGraphicsLinearLayout(Qt::Horizontal);
			group_it->m_scene_layout->addItem(layout);
			group_it->m_scene_layout->setAlignment(layout, m_internal->m_alignment);
			group_it->m_row_layouts << layout;
		}
		auto layout = group_it->m_row_layouts[row];

		// Add item.
		connect(&item, &EntityWidgetItem::signal_clicked, this, &EntityWidget::slot_clicked);
		layout->addItem(&item);
		m_internal->m_items << &item;
	}

	void EntityWidget::clear()
	{
		for (auto& entry : m_internal->m_groups)
		{
			delete entry.m_groupbox;
		}

		m_internal->m_groups.clear();
		m_internal->m_items.clear();
	}



	//================================================================================
	// Utility
	//================================================================================

	void EntityWidget::fit_layouts()
	{
		for (auto& group : m_internal->m_groups)
		{
			group.m_scene_layout->activate();
			group.m_scene->setSceneRect(0, 0, group.m_scene->itemsBoundingRect().width(), group.m_scene->itemsBoundingRect().height());
			group.m_groupbox->setFixedSize(group.m_scene->sceneRect().marginsAdded(QMarginsF(9.0f, 9.0f, 9.0f, 9.0f)).size().toSize());
			layout()->setAlignment(group.m_groupbox, Qt::AlignHCenter);
		}
	}



	//================================================================================
	// Events
	//================================================================================

	void EntityWidget::on_clicked(EntityWidgetItem* /*item*/, Qt::MouseButton /*button*/)
	{
	}



	//================================================================================
	// Accessors
	//================================================================================

	QVector<EntityWidgetItem*> EntityWidget::get_items()
	{
		return m_internal->m_items;	
	}

	QVector<const EntityWidgetItem*> EntityWidget::get_items() const
	{
		QVector<const EntityWidgetItem*> items;
		std::copy(m_internal->m_items.begin(), m_internal->m_items.end(), std::back_inserter(items));
		return items;
	}



	//================================================================================
	// Creators
	//================================================================================

	Result EntityWidget::create_item(const QJsonValue& json, EntityWidgetItem*& item)
	{
		EntityWidgetItemData data;
		auto data_result = create_data(json, data);
		if (!data_result)
		{
			return data_result;
		}

		item = new EntityWidgetItem({data});

		return Result();
	}

	Result EntityWidget::create_data(const QJsonValue& json, EntityWidgetItemData& data)
	{
		auto entity_string = json.toString();
		auto entity_parts = entity_string.split("|", QString::SkipEmptyParts);

		auto entity_name = (entity_parts.size() > 0 ? entity_parts[0] : QString());
		data.m_entity = m_internal->m_entity_db.get_entity(entity_name);
		if (data.m_entity == nullptr)
		{
			return Result(ResultType::Error, "Entity not found: " + entity_name);
		}

		auto attributes_string = (entity_parts.size() > 1 ? entity_parts[1] : QString());
		auto attributes_parts = attributes_string.split(",", QString::SkipEmptyParts);
		for (auto attribute : attributes_parts)
		{
			auto attribute_parts = attribute.split(":", QString::SkipEmptyParts);
			auto key = (attribute_parts.size() > 0 ? attribute_parts[0] : QString());
			auto value = (attribute_parts.size() > 1 ? attribute_parts[1] : QVariant());
			data.m_attributes.insert(key, value);
		}

		return Result();
	}



	//================================================================================
	// Item Slots
	//================================================================================

	void EntityWidget::slot_clicked(Qt::MouseButton button)
	{
		auto item = static_cast<EntityWidgetItem*>(sender());
		on_clicked(item, button);
		emit signal_clicked(item, button);
	}
}