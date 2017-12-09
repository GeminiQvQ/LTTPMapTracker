#ifndef EDITOR_INTERFACE_H
#define EDITOR_INTERFACE_H

// Forward declarations
namespace LTTPMapTracker
{
	class DataModel;
	class Settings;
}


namespace LTTPMapTracker
{
	class EditorInterface
	{
	public:
		virtual Settings&			get_settings	()			= 0;
		virtual const Settings&		get_settings	()	const	= 0;

		virtual DataModel&			get_data_model	()			= 0;
		virtual const DataModel&	get_data_model	()	const	= 0;
	};
}

#endif