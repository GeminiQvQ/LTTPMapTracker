#ifndef INSTANCE_RULE_PARSER_H
#define INSTANCE_RULE_PARSER_H

// Project includes
#include "Data/Instance/InstanceTypeInfo.h"
#include "Data/Schema/SchemaTypeInfo.h"
#include "Utility/Result.h"

// Forward declarations
namespace LTTPMapTracker
{
	enum class SchemaRuleTypeProgressSpecial;
}


namespace LTTPMapTracker
{
	// Rule
	bool match_rule(const Instance& instance, const SchemaRuleEntry& entry);
	bool match_rule(const Instance& instance, SchemaRuleTypeProgressSpecial special);
	bool match_rule(const Instance& instance, SchemaRuleCPtr rule);

	// Schema
	bool match_rule(const Instance& instance, SchemaItemCPtr schema_item);
	bool match_rule(const Instance& instance, SchemaRegionCPtr schema_region);

	// Instance
	bool match_rule(const Instance& instance, InstanceItemCPtr instance_item);
}

#endif