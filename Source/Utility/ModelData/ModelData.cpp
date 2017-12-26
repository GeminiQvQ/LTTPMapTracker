// Project includes
#include "Utility/ModelData/ModelData.h"
#include "Utility/ModelData/ModelDataEditor.h"


namespace Utility
{
	//================================================================================
	// Editor Creator
	//================================================================================

	ModelDataEditor* create_editor(const ModelDataEntry& data)
	{
		ModelDataEditor* editor = nullptr;

		auto type = data.get_value().type();

		if (type == QVariant::Bool)
		{
			editor = new MDEBool();
		}

		if (type == QVariant::Int)
		{
			if (data.has_attribute(ModelDataAttribute::Enum))
			{
				editor = new MDEIntComboBox();
			}
			else
			{
				editor = new MDEInt();
			}
		}

		if (type == QVariant::Double)
		{
			editor = new MDEFloat();
		}

		if (type == QVariant::String)
		{
			if (data.has_attribute(ModelDataAttribute::Enum))
			{
				editor = new MDEStringComboBox();
			}
			else if (data.has_attribute(ModelDataAttribute::File))
			{
				editor = new MDEStringFile();
			}
			else
			{
				editor = new MDEString();
			}
		}

		if (type == QVariant::Color)
		{
			editor = new MDEColor();
		}

		Q_ASSERT(editor != nullptr);
		editor->set_data(data);
		return editor;
	}
}


namespace Utility
{
	//================================================================================
	// Construction & Destruction
	//================================================================================

	ModelDataEntry::ModelDataEntry()
	{
	}

	ModelDataEntry::ModelDataEntry(const QVariant& value)
		: m_value(value)
	{
	}



	//================================================================================
	// Data
	//================================================================================

	void ModelDataEntry::set_value(const QVariant& value)
	{
		m_value = value;
	}

	const QVariant& ModelDataEntry::get_value() const
	{
		return m_value;
	}

	//--------------------------------------------------------------------------------

	void ModelDataEntry::set_attribute(ModelDataAttribute type, const QVariant& value)
	{
		m_attributes[type] = value;
	}

	const QVariant& ModelDataEntry::get_attribute(ModelDataAttribute type, const QVariant& value) const
	{
		auto it = m_attributes.find(type);
		return (it != m_attributes.end() ? *it : value);
	}

	bool ModelDataEntry::has_attribute(ModelDataAttribute type) const
	{
		return m_attributes.contains(type);
	}



	//================================================================================
	// Operators
	//================================================================================

	bool ModelDataEntry::operator==(const ModelDataEntry& rhs) const
	{
		return (get_value() == rhs.get_value());
	}

	bool ModelDataEntry::operator!=(const ModelDataEntry& rhs) const
	{
		return !(*this == rhs);
	}	
}


namespace Utility
{
	//================================================================================
	// Construction & Destruction
	//================================================================================

	ModelData::ModelData()
		: m_editor_creator(create_editor)
	{
	}



	//================================================================================
	// Data
	//================================================================================

	void ModelData::set_editor_creator(const ModelDataEditorCreator& creator)
	{
		m_editor_creator = creator;
	}

	const ModelDataEditorCreator& ModelData::get_editor_creator() const
	{
		return m_editor_creator;
	}
}