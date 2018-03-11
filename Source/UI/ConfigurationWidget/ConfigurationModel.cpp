// Project includes
#include "UI/ConfigurationWidget/ConfigurationModel.h"
#include "Data/Configuration.h"
#include "Data/Schema/Schema.h"
#include "Data/Settings.h"
#include "Utility/ModelData/ModelData.h"
#include "Utility/File.h"
#include "EditorInterface.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct ConfigurationModel::Internal
	{
		EditorInterface& m_editor_interface;
		ConfigurationPtr m_configuration;

		Internal(EditorInterface& editor_interface)
			: m_editor_interface(editor_interface)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	ConfigurationModel::ConfigurationModel(EditorInterface& editor_interface, QObject* parent)
		: QAbstractItemModel(parent)
		, m_internal(std::make_unique<Internal>(editor_interface))
	{
	}

	ConfigurationModel::~ConfigurationModel()
	{
	}



	//================================================================================
	// Configuration
	//================================================================================

	void ConfigurationModel::set_configuration(ConfigurationPtr configuration)
	{
		beginResetModel();
		m_internal->m_configuration = configuration;
		endResetModel();
	}

	void ConfigurationModel::clear_configuration()
	{
		beginResetModel();
		m_internal->m_configuration = nullptr;
		endResetModel();
	}



	//================================================================================
	// QAbstractItemModel Interface
	//================================================================================

	QModelIndex ConfigurationModel::index(int row, int column, const QModelIndex& /*parent*/) const
	{
		return createIndex(row, column, row);
	}

	QModelIndex ConfigurationModel::parent(const QModelIndex& /*child*/) const
	{
		return QModelIndex();
	}

	int ConfigurationModel::rowCount(const QModelIndex& parent) const
	{
		return (m_internal->m_configuration != nullptr && !parent.isValid() ? 6 : 0);
	}

	int ConfigurationModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 2;
	}

	Qt::ItemFlags ConfigurationModel::flags(const QModelIndex& index) const
	{
		int flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

		if (index.row() >= 2 && index.column() == 1)
		{
			flags |= Qt::ItemIsEditable;
		}
		
		return flags;
	}

	QVariant ConfigurationModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
	{
		if (role == Qt::DisplayRole)
		{
			switch (section)
			{
			case 0: return "Property";
			case 1: return "Value";
			}
		}
		
		return QVariant();
	}

	QVariant ConfigurationModel::data(const QModelIndex& index, int role) const
	{
		if (index.column() == 0 && role == Qt::DisplayRole)
		{
			switch (index.row())
			{
			case 0: return "Configuration";
			case 1: return "Schema";
			case 2: return "Name";
			case 3: return "Image";
			case 4: return "Instance Template";
			case 5: return "Settings Overrides";
			}
		}

		if (index.column() == 1)
		{
			if (index.row() == 0)
			{
				if (role == Qt::DisplayRole)
				{
					auto filename = get_relative_path(m_internal->m_configuration->get_filename());
					return (!filename.isEmpty() ? filename : "<Untitled>");
				}
			}

			if (index.row() == 1)
			{
				if (role == Qt::DisplayRole)
				{
					auto filename = get_relative_path(m_internal->m_configuration->get().m_schema->get_filename());
					return (!filename.isEmpty() ? filename : "<Untitled>");
				}
			}

			if (index.row() == 2)
			{
				if (role == Qt::DisplayRole || role == Qt::EditRole)
				{
					return m_internal->m_configuration->get().m_name;
				}
			}

			if (index.row() == 3)
			{
				if (role == Qt::DisplayRole)
				{
					return get_relative_path(m_internal->m_configuration->get().m_image);
				}

				if (role == ModelDataRole)
				{
					ModelData data;
					ModelDataEntry entry;
					entry.set_value(get_relative_path(m_internal->m_configuration->get().m_image));
					entry.set_attribute(ModelDataAttribute::File);
					entry.set_attribute(ModelDataAttribute::FileFilter, "PNG Files (*.png)");
					entry.set_attribute(ModelDataAttribute::FileDir, get_absolute_path("Data"));
					data << entry;

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 4)
			{
				if (role == Qt::DisplayRole)
				{
					return get_relative_path(m_internal->m_configuration->get().m_instance_template);
				}

				if (role == ModelDataRole)
				{
					ModelData data;
					ModelDataEntry entry;
					entry.set_value(get_relative_path(m_internal->m_configuration->get().m_instance_template));
					entry.set_attribute(ModelDataAttribute::File);
					entry.set_attribute(ModelDataAttribute::FileFilter, "Instance Files (*.instance.json)");
					entry.set_attribute(ModelDataAttribute::FileDir, get_absolute_path("Data/Instances/"));
					data << entry;

					return QVariant::fromValue(data);
				}
			}

			if (index.row() == 5)
			{
				if (role == Qt::DisplayRole)
				{
					return get_relative_path(m_internal->m_configuration->get().m_settings_overrides);
				}

				if (role == ModelDataRole)
				{
					ModelData data;
					ModelDataEntry entry;
					entry.set_value(get_relative_path(m_internal->m_configuration->get().m_settings_overrides));
					entry.set_attribute(ModelDataAttribute::File);
					entry.set_attribute(ModelDataAttribute::FileFilter, "Settings Files (*.ini)");
					entry.set_attribute(ModelDataAttribute::FileDir, get_absolute_path("Data/"));
					data << entry;

					return QVariant::fromValue(data);
				}
			}
		}

		return QVariant();
	}

	bool ConfigurationModel::setData(const QModelIndex& index, const QVariant& value, int role)
	{
		if (index.row() == 2 && role == Qt::EditRole)
		{
			auto data = m_internal->m_configuration->get();
			data.m_name = value.toString();
			m_internal->m_configuration->set(data);
		}

		if (index.row() == 3 && role == ModelDataRole)
		{
			auto data = m_internal->m_configuration->get();
			data.m_image = value.value<ModelData>()[0].get_value().toString();
			m_internal->m_configuration->set(data);
		}

		if (index.row() == 4 && role == ModelDataRole)
		{
			auto data = m_internal->m_configuration->get();
			data.m_instance_template = value.value<ModelData>()[0].get_value().toString();
			m_internal->m_configuration->set(data);
		}

		if (index.row() == 5 && role == ModelDataRole)
		{
			auto data = m_internal->m_configuration->get();
			data.m_settings_overrides = value.value<ModelData>()[0].get_value().toString();
			m_internal->m_configuration->set(data);

			m_internal->m_editor_interface.get_settings().reset();
		}

		return false;
	}
}