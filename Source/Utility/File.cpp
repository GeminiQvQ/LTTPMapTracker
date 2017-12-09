// Project includes
#include "Utility/File.h"

// Qt includes
#include <QApplication>
#include <QFileInfo>


namespace Utility
{
	//================================================================================
	// Path
	//================================================================================

	QString get_relative_path(QString path)
	{
		return (path.startsWith(QApplication::applicationDirPath()) ? path.mid(QApplication::applicationDirPath().length() + 1) : path);
	}

	QString get_absolute_path(QString path)
	{
		return (!path.isEmpty() && !QFileInfo(path).isAbsolute() ? QApplication::applicationDirPath() + "/" + path : path);
	}

	bool is_absolute_path(QString path)
	{
		return QFileInfo(path).isAbsolute();
	}

	bool is_relative_path(QString path)
	{
		return !is_absolute_path(path);
	}
}