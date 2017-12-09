#ifndef UTILITY_ENUM_REFLECTION_H
#define UTILITY_ENUM_REFLECTION_H

// Project includes
#include "Utility/Utility.h"

// Qt includes
#include <QVector>


namespace Utility
{
	template <typename T>
	struct EnumReflectionInfo
	{
		T		m_type;
		QString m_type_name;
		QString m_display_name;
	};

	template <typename T, typename Info = EnumReflectionInfo<T>>
	struct EnumReflection;
}

#define DECLARE_ENUM_REFLECTION(TYPE) \
	DECLARE_ENUM_REFLECTION_INFO(TYPE, Utility::EnumReflectionInfo<TYPE>)

#define DECLARE_ENUM_REFLECTION_INFO(TYPE, INFO)										\
template <>																				\
struct Utility::EnumReflection<TYPE, INFO>												\
{																						\
public:																					\
	static int			num		();														\
	static const INFO&	info	(int i);												\
	static const INFO&	info	(TYPE type);											\
	static const INFO*	info	(QString type_name);									\
	static int			index	(TYPE type);											\
																						\
private:																				\
	static const QVector<INFO> s_info;													\
};																						\

#define DEFINE_ENUM_REFLECTION(TYPE) \
	DEFINE_ENUM_REFLECTION_INFO(TYPE, Utility::EnumReflectionInfo<TYPE>)

#define DEFINE_ENUM_REFLECTION_INFO(TYPE, INFO)											\
int Utility::EnumReflection<TYPE, INFO>::num()											\
{																						\
	return s_info.size();																\
}																						\
																						\
const INFO& Utility::EnumReflection<TYPE, INFO>::info(int i)							\
{																						\
	Q_ASSERT(i >= 0 && i < s_info.size());												\
	return s_info[i];																	\
}																						\
																						\
const INFO& Utility::EnumReflection<TYPE, INFO>::info(TYPE type)						\
{																						\
	auto it = std::find_if(s_info.begin(), s_info.end(), [type] (const INFO & info)		\
	{																					\
		return (info.m_type == type);													\
	});																					\
	Q_ASSERT(it != s_info.end());														\
	return *it;																			\
}																						\
																						\
const INFO* Utility::EnumReflection<TYPE, INFO>::info(QString type_name)				\
{																						\
	auto it = std::find_if(s_info.begin(), s_info.end(), [type_name] (const INFO& info)	\
	{																					\
		return (info.m_type_name == type_name);											\
	});																					\
	return (it != s_info.end() ? &*it : nullptr);										\
}																						\
																						\
int Utility::EnumReflection<TYPE, INFO>::index(TYPE type)								\
{																						\
	auto it = std::find_if(s_info.begin(), s_info.end(), [type] (const INFO& info)		\
	{																					\
		return (info.m_type == type);													\
	});																					\
	return (it != s_info.end() ? std::distance(s_info.begin(), it) : -1);				\
}																						\
																						\
const QVector<INFO> Utility::EnumReflection<TYPE,INFO>::s_info =						\

#endif