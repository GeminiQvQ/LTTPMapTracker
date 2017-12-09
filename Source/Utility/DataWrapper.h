#ifndef UTILITY_DATA_WRAPPER_H
#define UTILITY_DATA_WRAPPER_H

// Project includes
#include "Utility/Result.h"
#include "Utility/Utility.h"

// Qt includes
#include <QObject>

// Stdlib includes
#include <memory>

// Forward declarations
class QJsonObject;


namespace Utility
{
	// Base
	//--------------------------------------------------------------------------------

	class DataWrapperBase : public QObject
						  , public std::enable_shared_from_this<DataWrapperBase>
	{
		Q_OBJECT

	signals:
		void signal_modified();
	};


	// Data Wrapper
	//--------------------------------------------------------------------------------

	template <typename T>
	class DataWrapper : public DataWrapperBase
	{
	public:
		const T& get() const
		{
			return m_data;
		}

		void set(const T& data)
		{
			m_data = data;
			emit signal_modified();
		}

	protected:
		T m_data;
	};


	// Serializable Data Wrapper
	//--------------------------------------------------------------------------------

	template <typename T>
	class SerializableDataWrapper : public DataWrapper<T>
	{
	public:
		template <typename... Args>
		void serialise(Args&&... args) const
		{
			m_data.serialise(std::forward<Args>(args)...);
		}

		template <typename... Args>
		Result deserialise(Args&&... args)
		{
			return m_data.deserialise(std::forward<Args>(args)...);
		}
	};
}

#endif