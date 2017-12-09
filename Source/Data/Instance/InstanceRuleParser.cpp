// Project includes
#include "Data/Instance/InstanceRuleParser.h"
#include "Data/Instance/Instance.h"
#include "Data/Schema/Schema.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Rule
	//================================================================================

	bool match_rule(const Instance& instance, const SchemaRuleEntry& entry)
	{
		if (entry.m_type == SchemaRuleType::ProgressItem)
		{
			auto& progress_items = instance.progress_items().get();
			auto it = std::find_if(progress_items.begin(), progress_items.end(), [&entry] (InstanceProgressItemCPtr progress_item)
			{
				return (progress_item->get().m_item->m_entity->m_type_name == entry.m_value);
			});
			return (it != progress_items.end());
		}

		if (entry.m_type == SchemaRuleType::ProgressLocation)
		{
			auto& progress_locations = instance.progress_locations().get();
			auto it = std::find_if(progress_locations.begin(), progress_locations.end(), [&entry] (InstanceProgressLocationCPtr progress_location)
			{
				return (progress_location->get().m_location->m_entity->m_type_name == entry.m_value);
			});
			return (it != progress_locations.end() && (*it)->get().m_cleared);
		}

		if (entry.m_type == SchemaRuleType::ProgressSpecial)
		{
			auto info = EnumReflection<SchemaRuleTypeProgressSpecial>::info(entry.m_value);
			return (info != nullptr && match_rule(instance, info->m_type));
		}

		if (entry.m_type == SchemaRuleType::SchemaRule)
		{
			auto rule = instance.get_schema()->rules().find(entry.m_value);
			return (rule != nullptr && match_rule(instance, rule));
		}

		if (entry.m_type == SchemaRuleType::SchemaItem)
		{
			auto schema_item = instance.get_schema()->items().find(entry.m_value);
			return (schema_item != nullptr && match_rule(instance, schema_item));
		}

		if (entry.m_type == SchemaRuleType::SchemaRegion)
		{
			auto schema_region = instance.get_schema()->regions().find(entry.m_value);
			return (schema_region != nullptr && match_rule(instance, schema_region));
		}

		if (entry.m_type == SchemaRuleType::Inaccessible)
		{
			return false;
		}

		return false;
	}
	
	bool match_rule(const Instance& instance, SchemaRuleTypeProgressSpecial special)
	{
		auto& progress_locations = instance.progress_locations().get();

		switch (special)
		{
		case SchemaRuleTypeProgressSpecial::Pendant1: 
			return (std::find_if(progress_locations.begin(), progress_locations.end(), [] (InstanceProgressLocationCPtr progress_location)
			{
				return (progress_location->get().m_is_pendant_green && progress_location->get().m_cleared);
			}) != progress_locations.end());

		case SchemaRuleTypeProgressSpecial::Pendant2: 
			return (std::count_if(progress_locations.begin(), progress_locations.end(), [] (InstanceProgressLocationCPtr progress_location)
			{
				return (progress_location->get().m_is_pendant && progress_location->get().m_cleared);
			}) >= 1);

		case SchemaRuleTypeProgressSpecial::Pendant3: 
			return (std::count_if(progress_locations.begin(), progress_locations.end(), [] (InstanceProgressLocationCPtr progress_location)
			{
				return (progress_location->get().m_is_pendant && progress_location->get().m_cleared);
			}) >= 2);

		case SchemaRuleTypeProgressSpecial::Crystal5: 
			return (std::count_if(progress_locations.begin(), progress_locations.end(), [] (InstanceProgressLocationCPtr progress_location)
			{
				return (progress_location->get().m_is_crystal_red && progress_location->get().m_cleared);
			}) >= 1);

		case SchemaRuleTypeProgressSpecial::Crystal6: 
			return (std::count_if(progress_locations.begin(), progress_locations.end(), [] (InstanceProgressLocationCPtr progress_location)
			{
				return (progress_location->get().m_is_crystal_red && progress_location->get().m_cleared);
			}) >= 2);

		case SchemaRuleTypeProgressSpecial::Crystal7: 
			return (std::count_if(progress_locations.begin(), progress_locations.end(), [] (InstanceProgressLocationCPtr progress_location)
			{
				return ((progress_location->get().m_is_crystal || progress_location->get().m_is_crystal_red) && progress_location->get().m_cleared);
			}) >= 7);
		}

		return false;
	}

	bool match_rule(const Instance& instance, SchemaRuleCPtr rule)
	{
		// Ensure we're not infinite looping.
		static QVector<SchemaRuleCPtr> rules;
		
		if (rules.contains(rule))
		{
			return false;
		}

		rules << rule;

		// Build an expression tree.
		struct ExpressionNode
		{
			SchemaRuleOperator						 m_operator;
			const SchemaRuleEntry*					 m_entry;
			QVector<std::shared_ptr<ExpressionNode>> m_children;
			std::weak_ptr<ExpressionNode>			 m_parent;

			ExpressionNode() : m_operator(SchemaRuleOperator::Or), m_entry(nullptr) {}
		};

		auto root = std::make_shared<ExpressionNode>();
		auto node = root;

		auto& entries = rule->get().m_entries;
		for (int entry_index = 0; entry_index < entries.size(); ++entry_index)
		{
			auto& entry = entries[entry_index];

			for (int i = 0; i < entry.m_brackets_open + 1; ++i)
			{
				if (node->m_children.size() == 2)
				{
					auto proxy_node = std::make_shared<ExpressionNode>();
					proxy_node->m_operator = entries[entry_index - 1].m_operator;

					auto parent = node->m_parent.lock();
					if (parent != nullptr)
					{
						parent->m_children.removeOne(node);
						parent->m_children << proxy_node;
					}

					proxy_node->m_parent = node->m_parent;
					node->m_parent = proxy_node;
					proxy_node->m_children << node;
					
					if (node == root)
					{
						root = proxy_node;
					}

					node = proxy_node;
				}

				auto new_node = std::make_shared<ExpressionNode>();
				node->m_children << new_node;
				new_node->m_parent = node;
				node = new_node;
			}

			node->m_entry = &entry;

			for (int i = 0; i < entry.m_brackets_close; ++i)
			{
				node = node->m_parent.lock();
			}

			auto parent = node->m_parent.lock();
			if (parent->m_children.size() == 1)
			{
				parent->m_operator = entry.m_operator;
			}

			node = parent;
		}

		// Evaluate the expression.
		std::function<bool(const ExpressionNode&)> evaluate = [&evaluate, &instance] (const ExpressionNode& node)
		{
			if (node.m_entry != nullptr)
			{
				return match_rule(instance, *node.m_entry);
			}
			else if (node.m_children.isEmpty())
			{
				return true;
			}
			else
			{
				auto result = evaluate(*node.m_children[0]);
				if (node.m_children.size() == 2)
				{
					auto result2 = evaluate(*node.m_children[1]);
					result = (node.m_operator == SchemaRuleOperator::Or ? result || result2 : result && result2);
				}
				return result;
			}
		};

		bool result = evaluate(*root);

		rules.removeOne(rule);

		return result;
	}



	//================================================================================
	// Schema
	//================================================================================

	bool match_rule(const Instance& instance, SchemaItemCPtr schema_item)
	{
		// Ensure we're not infinite looping.
		static QVector<SchemaItemCPtr> schema_items;
		
		if (schema_items.contains(schema_item))
		{
			return false;
		}

		schema_items << schema_item;

		if ((schema_item->get().m_region == nullptr || match_rule(instance, schema_item->get().m_region)) &&
			(schema_item->get().m_rule == nullptr || match_rule(instance, schema_item->get().m_rule)))
		{
			schema_items.removeOne(schema_item);
			return true;
		}

		auto& connections = instance.connections().get();
		for (auto connection : connections)
		{
			auto& items = connection->get().m_items;
			if (std::none_of(items.begin(), items.end(), [schema_item] (InstanceItemCPtr item)
			{
				return (item->get().m_schema_item == schema_item);
			}))
			{
				continue;
			}

			auto this_item = (items[0]->get().m_schema_item == schema_item ? items[0] : items[1]);
			auto other_item = (this_item == items[0] ? items[1] : items[0]);

			if (match_rule(instance, other_item->get().m_schema_item))
			{
				schema_items.removeOne(schema_item);
				return true;
			}
		}

		auto& items = instance.items();
		auto instance_item_it = std::find_if(items.begin(), items.end(), [schema_item] (InstanceItemCPtr item)
		{
			return (item->get().m_schema_item == schema_item);
		});
		Q_ASSERT(instance_item_it != items.end());
		auto instance_item = *instance_item_it;

		bool is_start_pos = (instance_item->get().m_location != nullptr && instance_item->get().m_location->m_is_startpos);
		if (is_start_pos)
		{
			schema_items.removeOne(schema_item);
			return true;
		}

		auto location = instance_item->get().m_location;
		if (location != nullptr && !location->m_entrances.isEmpty())
		{
			for (auto item : items)
			{
				auto other_location = item->get().m_location;

				if (item != instance_item && other_location == location && match_rule(instance, item))
				{
					schema_items.removeOne(schema_item);
					return true;
				}
			}
		}

		schema_items.removeOne(schema_item);

		return false;
	}

	bool match_rule(const Instance& instance, SchemaRegionCPtr schema_region)
	{
		if (schema_region->get().m_accessible_cached)
		{
			return schema_region->get().m_accessible;
		}

		auto& connections = instance.connections().get();
		QVector<SchemaRegionCPtr> regions;

		std::function<bool(SchemaRegionCPtr)> check_region = [&check_region, &instance, &connections, &regions] (SchemaRegionCPtr region)
		{
			if (regions.contains(region))
			{
				return false;
			}

			regions << region;

			if (region == nullptr || region->get().m_rule == nullptr || match_rule(instance, region->get().m_rule))
			{
				regions.removeOne(region);
				return true;
			}

			for (auto connection : connections)
			{
				auto& items = connection->get().m_items;
				if (std::none_of(items.begin(), items.end(), [region] (InstanceItemCPtr item)
				{
					return (item->get().m_schema_item->get().m_region == region);
				}))
				{
					continue;
				}

				auto this_item = (items[0]->get().m_schema_item->get().m_region == region ? items[0] : items[1]);
				auto other_item = (this_item == items[0] ? items[1] : items[0]);

				if (this_item->get().m_schema_item->get().m_region == other_item->get().m_schema_item->get().m_region)
				{
					regions.removeOne(region);
					return true;
				}

				auto this_rule = this_item->get().m_schema_item->get().m_rule;
				auto this_rule_access = this_item->get().m_schema_item->get().m_rule_access;

				if (this_item->get().m_schema_item->get().m_region == region && (this_rule == nullptr || this_rule_access == SchemaRuleAccessType::Entrance || match_rule(instance, this_rule)) && match_rule(instance, other_item))
				{
					regions.removeOne(region);
					return true;
				}
			}

			auto& items = instance.items();
			for (auto item : items)
			{
				auto location = item->get().m_location;

				if (location != nullptr && item->get().m_schema_item->get().m_region == region)
				{
					if (location->m_is_startpos)
					{
						auto rule = item->get().m_schema_item->get().m_rule;
						auto rule_access = item->get().m_schema_item->get().m_rule_access;
						
						if (rule == nullptr || rule_access == SchemaRuleAccessType::Entrance || match_rule(instance, rule))
						{
							regions.removeOne(region);
							return true;
						}
					}
				}

				if (location != nullptr && !location->m_connections.isEmpty() && item->get().m_schema_item->get().m_region == region)
				{
					auto it = std::find_if(items.begin(), items.end(), [location, item, region] (InstanceItemCPtr other_item)
					{
						return (other_item->get().m_location == location && other_item != item && other_item->get().m_schema_item->get().m_region != region);
					});

					if (it != items.end() && match_rule(instance, *it) && check_region((*it)->get().m_schema_item->get().m_region))
					{
						auto e1 = item->get().m_location_entrance;
						auto e2 = (*it)->get().m_location_entrance;

						QVector<LocationConnection> checked;
						std::function<bool(EntityCPtr)> check = [&check, &checked, location, e1, &instance] (EntityCPtr entrance)
						{
							for (auto& connection : location->m_connections)
							{
								if (!connection.m_entrances.contains(entrance))
								{
									continue;
								}

								auto is_checked = std::any_of(checked.begin(), checked.end(), [&connection] (const LocationConnection& connection_)
								{
									return (connection_.m_entrances == connection.m_entrances);
								});

								if (is_checked)
								{
									continue;
								}

								checked << connection;

								if (connection.m_entrances.contains(e1))
								{
									return true;
								}

								if (match_location_requirements(connection.m_requirements, instance) == LocationMatch::No)
								{
									continue;
								}

								if (check(connection.m_entrances[0] != entrance ? connection.m_entrances[0] : connection.m_entrances[1]))
								{
									return true;
								}
							}

							return false;
						};

						if (check(e2))
						{
							regions.removeOne(region);
							return true;
						}
					}
				}
			}

			regions.removeOne(region);

			return false;
		};

		return check_region(schema_region);
	}



	//================================================================================
	// Instance
	//================================================================================

	bool match_rule(const Instance& instance, InstanceItemCPtr instance_item)
	{
		return match_rule(instance, instance_item->get().m_schema_item);
	}
}