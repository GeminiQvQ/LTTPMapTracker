#ifndef UTILITY_RESULT_H
#define UTILITY_RESULT_H

// Project includes
#include "Utility/Utility.h"

// Qt includes
#include <QVector>

// Forward declarations
class QWidget;


namespace Utility
{
	// Result
	//--------------------------------------------------------------------------------

	enum class ResultType
	{
		Error,
		Warning,
		Ok
	};

	QString get_result_type_display_name(ResultType type);

	struct ResultEntry
	{
		ResultType	m_type;
		QString		m_message;

		ResultEntry();
		ResultEntry(bool result, QString message);
		ResultEntry(ResultType type, QString message);
	};

	using ResultEntryList = QVector<ResultEntry>;

	class Result
	{
	public:
		// Construction & Destruction
								Result			();
								Result			(bool result, QString message);
								Result			(ResultType type, QString message);

		// Accessors
		ResultType				get_type		() const;
		const ResultEntryList&	get_entries		() const;

		// Operators
		Result&					operator <<		(const Result& result);
		Result&					operator <<		(const ResultEntry& entry);
								operator bool	() const;

	private:
		ResultEntryList			m_entries;
	};


	// Utility
	//--------------------------------------------------------------------------------

	void report_result(const Result& result, QWidget* parent, QString title);
}

#endif