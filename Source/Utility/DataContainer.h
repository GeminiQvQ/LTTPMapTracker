#ifndef UTILITY_DATA_CONTAINER_H
#define UTILITY_DATA_CONTAINER_H

// Project includes
#include "Utility/DataWrapper.h"
#include "Utility/Result.h"
#include "Utility/Utility.h"

// Qt includes
#include <QObject>
#include <QVector>

// Stdlib includes
#include <functional>
#include <memory>

// Forward declarations
class QJsonObject;


namespace Utility
{
	// Base
	//--------------------------------------------------------------------------------

	class DataContainerBase : public QObject
						    , public std::enable_shared_from_this<DataContainerBase>
	{
		Q_OBJECT

	signals:
		void signal_to_be_added		(int index);
		void signal_added			(int index);
		void signal_to_be_removed	(int index);
		void signal_removed			(int index);
		void signal_modified		(int index);
		void signal_to_be_cleared	();
		void signal_cleared			();
	};


	// Data Container
	//--------------------------------------------------------------------------------

	template <typename T, typename... Args>
	class DataContainer : public DataContainerBase
	{
	public:
		using DataPtr = std::shared_ptr<T>;
		using DataCPtr = std::shared_ptr<const T>;
		using DataList = QVector<DataPtr>;
		using DataCList = QVector<DataCPtr>;
		using DataCreator = std::function<DataPtr(Args... args)>;
		using DataCreatorEmpty = std::function<DataPtr()>;

		DataContainer(DataCreator creator, DataCreatorEmpty creator_empty)
			: m_creator(creator)
			, m_creator_empty(creator_empty)
		{
		}

		DataPtr add(Args... args)
		{
			auto data = m_creator(args...);
			int index = m_data.size();
			emit signal_to_be_added(index);
			m_data << data;
			m_cdata << data;
			connect(data.get(), &DataWrapperBase::signal_modified, this, [this, data] () { emit signal_modified(m_data.indexOf(data)); });
			emit signal_added(index);
			return data;
		}

		void remove(DataPtr data)
		{
			Q_ASSERT(m_data.contains(data));
			int index = m_data.indexOf(data);
			emit signal_to_be_removed(index);
			m_data.removeAt(index);
			m_cdata.removeAt(index);
			data->disconnect(this);
			emit signal_removed(index);
		}

		void clear()
		{
			emit signal_to_be_cleared();
			m_data.clear();
			m_cdata.clear();
			emit signal_cleared();
		}

		const QVector<DataPtr>& get()
		{
			return m_data;
		}

		const QVector<DataCPtr>& get() const
		{
			return m_cdata;
		}

		DataPtr operator[](int index)
		{
			return m_data[index];
		}

		DataCPtr operator[](int index) const
		{
			return m_data[index];
		}

		template <typename... Args>
		void serialise(QString name, QJsonObject& json, Args&&... args)
		{
			QJsonArray json_data_list;
			for (auto data : m_data)
			{
				QJsonObject json_data;
				data->serialise(json_data, std::forward<Args>(args)...);
				json_data_list << json_data;
			}
			json[name] = json_data_list;
		};

		template <typename... Args>
		Result deserialise(QString name, const QJsonObject& json, int version, Args&&... args)
		{
			m_data.clear();
			m_cdata.clear();

			Result result;

			QJsonValue jval_data_list;
			result << json_read(json, name, jval_data_list);
			auto json_data_list = jval_data_list.toArray();

			for (auto json_data : json_data_list)
			{
				auto data = m_creator_empty();
				auto data_result = data->deserialise(json_data.toObject(), version, std::forward<Args>(args)...);
				result << data_result;

				if (data_result)
				{
					connect(data.get(), &DataWrapperBase::signal_modified, this, [this, data] () { emit signal_modified(m_data.indexOf(data)); });
					m_data << data;
					m_cdata << data;
				}
			}

			return result;
		};

	protected:
		QVector<DataPtr>  m_data;
		QVector<DataCPtr> m_cdata;
		DataCreator		  m_creator;
		DataCreatorEmpty  m_creator_empty;
	};


	// Searchable Data Container
	//--------------------------------------------------------------------------------

	template <typename T, typename SearchType, typename... Args>
	class SearchableDataContainer : public DataContainer<T, Args...>
	{
	public:
		using SearchDelegate = std::function<bool(DataCPtr, SearchType)>;

		SearchableDataContainer(DataCreator creator, DataCreatorEmpty creator_empty, SearchDelegate search_delegate)
			: DataContainer<T, Args...>(creator, creator_empty)
			, m_search_delegate(search_delegate)
		{
		}

		DataPtr find(const SearchType& id)
		{
			return std::const_pointer_cast<T>(static_cast<const SearchableDataContainer<T, SearchType, Args...>*>(this)->find(id));
		}

		DataCPtr find(const SearchType& id) const
		{
			auto it = std::find_if(m_data.begin(), m_data.end(), [this, &id] (DataCPtr data)
			{
				return m_search_delegate(data, id);
			});
			return (it != m_data.end() ? *it : nullptr);
		}

		bool contains(const SearchType& id) const
		{
			return (find(id) != nullptr);
		}

	private:
		SearchDelegate m_search_delegate;
	};
}

#endif