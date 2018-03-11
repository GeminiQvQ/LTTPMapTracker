#ifndef STARTUP_MODEL_H
#define STARTUP_MODEL_H

// Project includes
#include "EditorTypeInfo.h"

// Qt includes
#include <QAbstractListModel>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class StartupModel : public QAbstractListModel
	{
	public:
		// Construction & Destruction
								StartupModel	(const DataModel& data_model, QObject* parent = nullptr);
								~StartupModel	();

		// QAbstractItemModel Interface
		virtual int				rowCount		(const QModelIndex& parent = QModelIndex())				const	override;
		virtual Qt::ItemFlags	flags			(const QModelIndex& index)								const	override;
		virtual QVariant		data			(const QModelIndex& index, int role = Qt::DisplayRole)	const	override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif