#ifndef UTILITY_FILE_BROWSE_WIDGET_H
#define UTILITY_FILE_BROWSE_WIDGET_H

// Qt includes
#include <QWidget>

// Stdlib includes
#include <memory>

namespace Utility
{
	class FileBrowseWidget : public QWidget
	{
		Q_OBJECT

	public:
		// Construction & Destruction
				FileBrowseWidget	(QString title, QString dir, QString filter, QWidget* parent = nullptr);
				~FileBrowseWidget	();

		// Properties
		void	set_filename		(QString filename);
		QString	get_filename		() const;

	private slots:
		// UI Slots
		void	slot_browse			();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif