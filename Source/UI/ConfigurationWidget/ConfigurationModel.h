#ifndef CONFIGURATION_MODEL_H
#define CONFIGURATION_MODEL_H

// Project includes
#include "Data/Configuration.h"

// Qt includes
#include <QAbstractItemModel>

// Stdlib includes
#include <memory>


namespace LTTPMapTracker
{
	class ConfigurationModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		// Construction & Destruction
								ConfigurationModel	(EditorInterface& editor_interface, QObject* parent = nullptr);
								~ConfigurationModel	();

		// Configuration
		void					set_configuration	(ConfigurationPtr configuration);
		void					clear_configuration	();

		// QAbstractItemModel Interface
		virtual QModelIndex		index				(int row, int column, const QModelIndex& parent = QModelIndex())			const	override;
		virtual QModelIndex		parent				(const QModelIndex& child)													const	override;
		virtual int				rowCount			(const QModelIndex& parent = QModelIndex())									const	override;
		virtual int				columnCount			(const QModelIndex& parent = QModelIndex())									const	override;
		virtual Qt::ItemFlags	flags				(const QModelIndex& index)													const	override;
		virtual QVariant		headerData			(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)		const	override;
		virtual QVariant		data				(const QModelIndex& index, int role = Qt::DisplayRole)						const	override;
		virtual bool			setData				(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)			override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif