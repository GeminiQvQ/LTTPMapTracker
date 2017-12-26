#ifndef UTILITY_MODEL_DATA_EDITOR_H
#define UTILITY_MODEL_DATA_EDITOR_H

// Project includes
#include "Utility/ModelData/ModelData.h"
#include "Utility/Utility.h"

// Qt includes
#include <QLayout>
#include <QWidget>

// Stdlib includes
#include <memory>


namespace Utility
{
	// Base
	//--------------------------------------------------------------------------------

	class ModelDataEditor : public QWidget
	{
		Q_OBJECT

	public:
		// Construction & Destruction
						ModelDataEditor		(QLayout* layout = new QHBoxLayout(), QWidget* parent = nullptr);

		// Data
		virtual void	set_data			(const ModelDataEntry& entry) = 0;
		virtual void	get_data			(ModelDataEntry& entry) const = 0;

	signals:
		// Signals
		void			signal_data_changed	();

	protected slots:
		// Data Slots
		virtual void	data_changed		();
	};


	// Model Data
	//--------------------------------------------------------------------------------

	class MDEModelData : public ModelDataEditor
	{
		Q_OBJECT

	public:
		// Construction & Destruction
		MDEModelData();
		~MDEModelData();

		// Data
		void				set_data		(const ModelData& data);
		const ModelData&	get_data		() const;

		virtual void		set_data		(const ModelDataEntry& entry) override;
		virtual void		get_data		(ModelDataEntry& entry) const override;

	protected:
		// Data Slots
		virtual void		data_changed	();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};


	// Bool
	//--------------------------------------------------------------------------------

	class MDEBool : public ModelDataEditor
	{
	public:
		MDEBool();
		~MDEBool();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};


	// Int
	//--------------------------------------------------------------------------------

	class MDEInt : public ModelDataEditor
	{
	public:
		MDEInt();
		~MDEInt();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};

	//--------------------------------------------------------------------------------

	class MDEIntComboBox : public ModelDataEditor
	{
	public:
		MDEIntComboBox();
		~MDEIntComboBox();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};


	// Float
	//--------------------------------------------------------------------------------

	class MDEFloat : public ModelDataEditor
	{
	public:
		MDEFloat();
		~MDEFloat();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};


	// String
	//--------------------------------------------------------------------------------

	class MDEString : public ModelDataEditor
	{
	public:
		MDEString();
		~MDEString();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};

	//--------------------------------------------------------------------------------

	class MDEStringComboBox : public ModelDataEditor
	{
	public:
		MDEStringComboBox();
		~MDEStringComboBox();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};

	//--------------------------------------------------------------------------------

	class MDEStringFile : public ModelDataEditor
	{
		Q_OBJECT

	public:
		MDEStringFile();
		~MDEStringFile();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private slots:
		void			slot_browse	();

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};


	// Color
	//--------------------------------------------------------------------------------

	class MDEColor : public ModelDataEditor
	{
	public:
		MDEColor();
		~MDEColor();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};	


	// Button
	//--------------------------------------------------------------------------------

	class MDEButton : public ModelDataEditor
	{
	public:
		MDEButton();
		~MDEButton();

		virtual void	set_data	(const ModelDataEntry& entry) override;
		virtual void	get_data	(ModelDataEntry& entry) const override;

	private:
		struct Internal;
		const std::unique_ptr<Internal> m_internal;
	};
}

#endif