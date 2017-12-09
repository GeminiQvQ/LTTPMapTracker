#ifndef EDITOR_TYPE_INFO_H
#define EDITOR_TYPE_INFO_H

// Project includes
#include "Data/Schema/SchemaTypeInfo.h"
#include "Data/Instance/InstanceTypeInfo.h"

// Forward declarations
namespace LTTPMapTracker
{
	class EditorInterface;
	class DataModel;
}


namespace LTTPMapTracker
{
	class Configuration;
	using ConfigurationPtr = std::shared_ptr<Configuration>;
	using ConfigurationCPtr = std::shared_ptr<const Configuration>;
}

#endif