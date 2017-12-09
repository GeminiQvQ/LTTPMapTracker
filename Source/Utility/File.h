#ifndef UTILITY_FILE_H
#define UTILITY_FILE_H

// Qt includes
#include <QString>


namespace Utility
{
	// Path
	QString get_relative_path	(QString path);
	QString get_absolute_path	(QString path);

	bool	is_absolute_path	(QString path);
	bool	is_relative_path	(QString path);
}

#endif