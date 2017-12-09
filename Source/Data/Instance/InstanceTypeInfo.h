#ifndef INSTANCE_TYPE_INFO_H
#define INSTANCE_TYPE_INFO_H

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class Instance;
	using InstancePtr = std::shared_ptr<Instance>;
	using InstanceCPtr = std::shared_ptr<const Instance>;

	class InstanceItem;
	using InstanceItemPtr = std::shared_ptr<InstanceItem>;
	using InstanceItemCPtr = std::shared_ptr<const InstanceItem>;

	class InstanceConnection;
	using InstanceConnectionPtr = std::shared_ptr<InstanceConnection>;
	using InstanceConnectionCPtr = std::shared_ptr<const InstanceConnection>;

	class InstanceProgressItem;
	using InstanceProgressItemPtr = std::shared_ptr<InstanceProgressItem>;
	using InstanceProgressItemCPtr = std::shared_ptr<const InstanceProgressItem>;

	class InstanceProgressLocation;
	using InstanceProgressLocationPtr = std::shared_ptr<InstanceProgressLocation>;
	using InstanceProgressLocationCPtr = std::shared_ptr<const InstanceProgressLocation>;
}

#endif