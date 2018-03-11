#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Project includes
#include "Utility/DataWrapper.h"
#include "EditorTypeInfo.h"


namespace LTTPMapTracker
{
	// Configuration Data
	//--------------------------------------------------------------------------------

	struct ConfigurationData
	{
		QString		m_name;
		QString		m_image;
		SchemaPtr	m_schema;
		InstancePtr m_instance;
		QString		m_instance_template;
		QString		m_settings_overrides;
		
				ConfigurationData	();
		void	serialise			(QJsonObject& json) const;
		Result	deserialise			(const QJsonObject& json, int version);
	};


	// Configuration
	//--------------------------------------------------------------------------------

	class Configuration : public SerializableDataWrapper<ConfigurationData>
	{
	public:
		// Construction & Destruction
				Configuration		(const DataModel& data_model);
				~Configuration		();

		// Save & Load
		Result	save				();
		Result	save				(QString filename);
		Result	load				(QString filename);

		// Properties
		QString	get_filename		();

		// Instance
		Result	create_instance		();
		void	destroy_instance	();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;	
	};
}

#endif