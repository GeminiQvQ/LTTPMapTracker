// Project includes
#include "Utility/Result.h"

// Qt includes
#include <QDialog>
#include <QMessageBox>
#include <QLayout>
#include <QListWidget>
#include <QWidget>


namespace Utility
{
	//================================================================================
	// ResultType
	//================================================================================

	QString get_result_type_display_name(ResultType type)
	{
		switch (type)
		{
		case ResultType::Error:   return "Error";
		case ResultType::Warning: return "Warning";
		case ResultType::Ok:	  return "Information";
		default:				  return QString();
		}
	}



	//================================================================================
	// ResultEntry
	//================================================================================

	ResultEntry::ResultEntry()
		: m_type(ResultType::Ok)
	{
	}

	ResultEntry::ResultEntry(bool result, QString message)
		: m_type(result ? ResultType::Ok : ResultType::Error)
		, m_message(message)
	{
	}

	ResultEntry::ResultEntry(ResultType type, QString message)
		: m_type(type)
		, m_message(message)
	{
	}



	//================================================================================
	// Result
	//================================================================================

	Result::Result()
	{
	}

	Result::Result(bool result, QString message)
	{
		*this << ResultEntry(result, message);
	}

	Result::Result(ResultType type, QString message)
	{
		*this << ResultEntry(type, message);
	}

	//--------------------------------------------------------------------------------

	ResultType Result::get_type() const
	{
		auto it = std::min_element(m_entries.begin(), m_entries.end(), [] (const ResultEntry& a, const ResultEntry& b)
		{
			return (a.m_type < b.m_type);
		});
		return (it != m_entries.end() ? it->m_type : ResultType::Ok);
	}

	const QVector<ResultEntry>& Result::get_entries() const
	{
		return m_entries;
	}

	//--------------------------------------------------------------------------------

	Result& Result::operator <<(const Result& result)
	{
		m_entries << result.m_entries;
		return *this;
	}

	Result& Result::operator <<(const ResultEntry& entry)
	{
		m_entries << entry;
		return *this;
	}

	Result::operator bool() const
	{
		return (get_type() != ResultType::Error);
	}



	//================================================================================
	// Utility
	//================================================================================

	void report_result(const Result& result, QWidget* parent, QString title)
	{
		auto type = result.get_type();

		if (type != ResultType::Ok)
		{
			if (std::all_of(result.get_entries().begin(), result.get_entries().end(), [] (const ResultEntry& entry)
			{
				return entry.m_message.isEmpty();
			}))
			{
				return;
			}

			if (result.get_entries().size() == 1)
			{
				auto& entry = result.get_entries()[0];

				switch (type)
				{
				case ResultType::Warning: QMessageBox::warning(parent, title, entry.m_message); break;
				case ResultType::Error: QMessageBox::critical(parent, title, entry.m_message); break;
				}
			}
			else
			{
				QDialog d(parent);
				d.setWindowTitle(title);
				auto l = new QHBoxLayout();
				l->setContentsMargins(0, 0, 0, 0);
				d.setLayout(l);

				auto list_widget = new QListWidget();
				l->addWidget(list_widget);

				for (auto& entry : result.get_entries())
				{
					list_widget->addItem(QString("[%1] %2").arg(get_result_type_display_name(entry.m_type)).arg(entry.m_message));
				}

				d.exec();
			}
		}
	}
}