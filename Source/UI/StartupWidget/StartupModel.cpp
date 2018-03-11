// Project includes
#include "UI/StartupWidget/StartupModel.h"
#include "Data/Configuration.h"

// Qt includes
#include <QDir>


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct StartupModel::Internal
	{
		struct Entry
		{
			QString m_filename;
			QString m_name;
			QPixmap m_image;
		};

		QVector<Entry> m_entries;
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	StartupModel::StartupModel(const DataModel& data_model, QObject* parent)
		: QAbstractListModel(parent)
		, m_internal(std::make_unique<Internal>())
	{
		QDirIterator it("Data/Configurations/", QStringList() << "*.configuration.json", QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext())
		{
			Configuration configuration(data_model);
			if (configuration.load(it.next()))
			{
				auto image = QPixmap(configuration.get().m_image);
				if (image.isNull())
				{
					image = QPixmap("Data/Images/Misc/NoImage.png");
				}

				Internal::Entry entry;
				entry.m_filename = configuration.get_filename();
				entry.m_name = configuration.get().m_name;
				entry.m_image = image;
				m_internal->m_entries << entry;
			}
		}
	}

	StartupModel::~StartupModel()
	{
	}

	

	//================================================================================
	// QAbstractItemModel Interface
	//================================================================================

	int StartupModel::rowCount(const QModelIndex& parent) const
	{
		return (!parent.isValid() ? m_internal->m_entries.size() : 0);
	}

	Qt::ItemFlags StartupModel::flags(const QModelIndex& /*index*/) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant StartupModel::data(const QModelIndex& index, int role) const
	{
		auto& entry = m_internal->m_entries[index.row()];

		if (role == Qt::DisplayRole)
		{
			return (!entry.m_name.isEmpty() ? entry.m_name : "<Untitled>");
		}

		if (role == Qt::ToolTipRole)
		{
			return entry.m_filename;
		}

		if (role == Qt::DecorationRole)
		{
			return entry.m_image;
		}

		if (role == Qt::UserRole)
		{
			return entry.m_filename;
		}

		return QVariant();
	}
}