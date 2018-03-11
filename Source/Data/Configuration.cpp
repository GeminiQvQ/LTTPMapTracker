// Project includes
#include "Data/Configuration.h"
#include "Data/Instance/Instance.h"
#include "Data/Schema/Schema.h"
#include "Utility/File.h"
#include "Utility/JSON.h"


namespace LTTPMapTracker
{
	//================================================================================
	// Configuration Data
	//================================================================================

	ConfigurationData::ConfigurationData()
		: m_schema(std::make_shared<Schema>())
	{
	}

	void ConfigurationData::serialise(QJsonObject& json) const
	{
		json["Schema"] = get_relative_path(m_schema->get_filename());
		json["Name"] = m_name;
		json["Image"]= get_relative_path(m_image);
		json["InstanceTemplate"] = get_relative_path(m_instance_template);
		json["SettingsOverrides"] = get_relative_path(m_settings_overrides);
	}

	Result ConfigurationData::deserialise(const QJsonObject& json, int /*version*/)
	{
		Result result;

		QString schema_filename;
		result << json_read(json, "Schema", schema_filename, QString());

		if (!schema_filename.isEmpty())
		{
			result << m_schema->load(get_absolute_path(schema_filename));
		}
		
		result << json_read(json, "Name", m_name, QString());

		QString image_filename;
		result << json_read(json, "Image", image_filename, QString());
		m_image = get_absolute_path(image_filename);

		QString instance_template_filename;
		result << json_read(json, "InstanceTemplate", instance_template_filename, QString());
		m_instance_template = get_absolute_path(instance_template_filename);

		QString settings_overrides_filename;
		result << json_read(json, "SettingsOverrides", settings_overrides_filename, QString());
		m_settings_overrides = get_absolute_path(settings_overrides_filename);

		return result;
	}
}


namespace LTTPMapTracker
{
	//================================================================================
	// Internal
	//================================================================================

	struct Configuration::Internal
	{
		const DataModel& m_data_model;
		QString			 m_filename;
		
		Internal(const DataModel& data_model)
			: m_data_model(data_model)
		{
		}
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	Configuration::Configuration(const DataModel& data_model)
		: m_internal(std::make_unique<Internal>(data_model))
	{
	}

	Configuration::~Configuration()
	{
	}



	//================================================================================
	// Save & Load
	//================================================================================

	Result Configuration::load(QString filename)
	{
		// Load data.
		QJsonObject json;
		
		auto load_result = json_load(json, filename);
		if (!load_result)
		{
			return load_result;
		}

		// Deserialize.
		int version = 0;
		if (!json_read(json, "Version", version, 0))
		{
			return Result(false, "Unable to read version.");
		}

		auto deserialise_result = deserialise(json, version);
		if (!deserialise_result)
		{
			return deserialise_result;
		}

		m_internal->m_filename = filename;

		return Result();
	}

	Result Configuration::save()
	{
		return (!m_internal->m_filename.isEmpty() ? save(m_internal->m_filename) : Result(ResultType::Error, "No filename set."));
	}

	Result Configuration::save(QString filename)
	{
		// Serialize.
		QJsonObject json;
		json["Version"] = 1;

		serialise(json);

		// Write data.
		auto save_result = json_save(json, filename);
		if (!save_result)
		{
			return save_result;
		}

		m_internal->m_filename = filename;

		return Result();
	}



	//================================================================================
	// Properties
	//================================================================================

	QString Configuration::get_filename()
	{
		return m_internal->m_filename;
	}



	//================================================================================
	// Instance
	//================================================================================

	Result Configuration::create_instance()
	{
		Result result;

		m_data.m_instance = std::make_shared<Instance>(m_internal->m_data_model, m_data.m_schema);

		if (!m_data.m_instance_template.isEmpty())
		{
			result << m_data.m_instance->load_template(m_data.m_instance_template);
		}

		return result;
	}

	void Configuration::destroy_instance()
	{
		m_data.m_instance = nullptr;
	}
}