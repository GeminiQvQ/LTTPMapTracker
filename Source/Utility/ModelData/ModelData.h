#ifndef UTILITY_MODEL_DATA_H
#define UTILITY_MODEL_DATA_H

// Project includes
#include "Utility/Utility.h"

// Qt includes
#include <QVariant>
#include <QVector>


namespace Utility
{
	// Types
	//--------------------------------------------------------------------------------

	enum ModelDataModelRole
	{
		ModelDataRole = Qt::UserRole
	};

	class ModelDataEntry;
	class ModelDataEditor;

	using ModelDataEditorCreator = std::function<ModelDataEditor*(const ModelDataEntry& data)>;


	// Attributes
	//--------------------------------------------------------------------------------
	
	enum class ModelDataAttribute
	{					//	[Target]		[Type]			[Description]
		MinSize,		//	Any				QSize			Minimum editor size.
		MaxSize,		//	Any				QSize			Maximum editor size.
		FixedSize,		//	Any				QSize			Fixed editor size.
		MinValue,		//	Int				int				Minimum editor value.
		MaxValue,		//	Int				int				Maximum editor value.
		Enum,			//	Int, String		QStringList		Provides a ComboBox widget for an index or a string value.
		EnumSorted,		//	Int, String		-				Enable sorting for ComboBox widgets.
		StringLabel,	//	String			-				Provides a Label widget for a string value.
		Button,			//	Int				-				Provides a button that invokes setData with a function id.
		ButtonLabel,	//	Int				QString			Label for data with the 'Button' attribute.
		File,			//	String			-				Provides a file browse widget for a string value.
		FileFilter,		//	String			QString			File browse filter.
		FileDir,		//	String			QString			File browse directory.

		Custom1,		// -				-				Custom attribute #1.
		Custom2,		// -				-				Custom attribute #2.
		Custom3,		// -				-				Custom attribute #3.
		Custom4,		// -				-				Custom attribute #4.
		Custom5,		// -				-				Custom attribute #5.
	};

	using ModelDataAttributeMap = QMap<ModelDataAttribute, QVariant>;


	// Model Data Entry
	//--------------------------------------------------------------------------------

	class ModelDataEntry
	{
	public:
		// Construction & Destruction
								ModelDataEntry	();
								ModelDataEntry	(const QVariant& value);

		// Data
		void					set_value		(const QVariant& value);
		const QVariant&			get_value		()																const;

		void					set_attribute	(ModelDataAttribute type, const QVariant& value = QVariant());
		const QVariant&			get_attribute	(ModelDataAttribute type, const QVariant& value = QVariant())	const;
		bool					has_attribute	(ModelDataAttribute type)										const;

		// Operators
		bool					operator ==		(const ModelDataEntry& rhs)										const;
		bool					operator !=		(const ModelDataEntry& rhs)										const;

	private:
		QVariant				m_value;
		ModelDataAttributeMap	m_attributes;
	};


	// Model Data
	//--------------------------------------------------------------------------------

	class ModelData : public QVector<ModelDataEntry>
	{
	public:
		// Construction & Destruction
										ModelData			();

		// Data
		void							set_editor_creator	(const ModelDataEditorCreator& creator);
		const ModelDataEditorCreator&	get_editor_creator	() const;

	private:
		ModelDataEditorCreator			m_editor_creator;
	};
}

Q_DECLARE_METATYPE(Utility::ModelData);

#endif