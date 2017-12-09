#ifndef SCHEMA_TYPE_INFO_H
#define SCHEMA_TYPE_INFO_H

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class Schema;
	using SchemaPtr = std::shared_ptr<Schema>;
	using SchemaCPtr = std::shared_ptr<const Schema>;

	class SchemaItem;
	using SchemaItemPtr = std::shared_ptr<SchemaItem>;
	using SchemaItemCPtr = std::shared_ptr<const SchemaItem>;

	class SchemaRule;
	using SchemaRulePtr = std::shared_ptr<SchemaRule>;
	using SchemaRuleCPtr = std::shared_ptr<const SchemaRule>;
	struct SchemaRuleEntry;

	class SchemaRegion;
	using SchemaRegionPtr = std::shared_ptr<SchemaRegion>;
	using SchemaRegionCPtr = std::shared_ptr<const SchemaRegion>;
}

#endif