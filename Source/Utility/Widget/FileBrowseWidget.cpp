// Project includes
#include "Utility/Widget/FileBrowseWidget.h"
#include "Utility/File.h"

// Qt includes
#include <QFileDialog>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>


namespace Utility
{
	//================================================================================
	// Internal
	//================================================================================

	struct FileBrowseWidget::Internal
	{
		QLineEdit*	 m_lineedit;
		QPushButton* m_browse_button;

		QString		 m_title;
		QString		 m_dir;
		QString		 m_filter;

		Internal(QString title, QString dir, QString filter)
			: m_lineedit(nullptr)
			, m_browse_button(nullptr)
			, m_title(title)
			, m_dir(get_absolute_path(dir))
			, m_filter(filter)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	FileBrowseWidget::FileBrowseWidget(QString title, QString dir, QString filter, QWidget* parent)
		: QWidget(parent)
		, m_internal(std::make_unique<Internal>(title, dir, filter))
	{
		auto layout = new QHBoxLayout();
		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);

		m_internal->m_lineedit = new QLineEdit();
		layout->addWidget(m_internal->m_lineedit);

		m_internal->m_browse_button = new QPushButton("...");
		connect(m_internal->m_browse_button, &QPushButton::clicked, this, &FileBrowseWidget::slot_browse);
		layout->addWidget(m_internal->m_browse_button);
	}

	FileBrowseWidget::~FileBrowseWidget()
	{
	}



	//================================================================================
	// Properties
	//================================================================================

	void FileBrowseWidget::set_filename(QString filename)
	{
		m_internal->m_lineedit->setText(get_relative_path(filename));
	}

	QString FileBrowseWidget::get_filename() const
	{
		return m_internal->m_lineedit->text();
	}



	//================================================================================
	// UI Slots
	//================================================================================

	void FileBrowseWidget::slot_browse()
	{
		auto current_filename = get_filename();
		auto dir = (!current_filename.isEmpty() ? get_absolute_path(current_filename).section("/", 0, -2) : m_internal->m_dir);

		auto filename = QFileDialog::getOpenFileName(this, m_internal->m_title, dir, m_internal->m_filter, nullptr, QFileDialog::DontResolveSymlinks);
		if (!filename.isEmpty())
		{
			set_filename(filename);
		}
	}
}