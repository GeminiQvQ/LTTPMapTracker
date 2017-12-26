// Project includes
#include "Utility/ModelData/ModelDataEditor.h"
#include "Utility/Widget/ColorButtonWidget.h"
#include "Utility/File.h"

// Qt includes
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>


namespace Utility
{
	//================================================================================
	// Base
	//================================================================================

	ModelDataEditor::ModelDataEditor(QLayout* layout, QWidget* parent)
		: QWidget(parent)
	{
		layout->setContentsMargins(1, 1, 1, 1);
		layout->setSpacing(1);
		setLayout(layout);
	}

	void ModelDataEditor::data_changed()
	{
		emit signal_data_changed();
	}



	//================================================================================
	// Model Data
	//================================================================================

	struct MDEModelData::Internal
	{
		ModelData				  m_data;
		QVector<ModelDataEditor*> m_editors;
	};

	//--------------------------------------------------------------------------------

	MDEModelData::MDEModelData()
		: m_internal(std::make_unique<Internal>())
	{
	}

	MDEModelData::~MDEModelData()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEModelData::set_data(const ModelData& data)
	{
		m_internal->m_data = data;

		if (data.size() != m_internal->m_editors.size())
		{
			for (auto editor : m_internal->m_editors)
			{
				editor->setParent(nullptr);
				editor->setVisible(false);
				editor->deleteLater();
			}

			m_internal->m_editors.clear();

			for (auto& entry : data)
			{
				auto editor = data.get_editor_creator()(entry);
				
				editor->setMinimumSize(entry.get_attribute(ModelDataAttribute::MinSize, editor->minimumSize()).toSize());
				editor->setMaximumSize(entry.get_attribute(ModelDataAttribute::MaxSize, editor->maximumSize()).toSize());

				if (entry.has_attribute(ModelDataAttribute::FixedSize))
				{
					editor->setFixedSize(entry.get_attribute(ModelDataAttribute::FixedSize).toSize());
				}

				connect(editor, &ModelDataEditor::signal_data_changed, this, &MDEModelData::data_changed);
				layout()->addWidget(editor);
				m_internal->m_editors << editor;
			}
		}

		for (int i = 0; i < data.size(); ++i)
		{
			m_internal->m_editors[i]->blockSignals(true);
			m_internal->m_editors[i]->set_data(data[i]);
			m_internal->m_editors[i]->blockSignals(false);
		}
	}

	const ModelData& MDEModelData::get_data() const
	{
		return m_internal->m_data;
	}

	//--------------------------------------------------------------------------------

	void MDEModelData::set_data(const ModelDataEntry& entry)
	{
		set_data(entry.get_value().value<ModelData>());
	}

	void MDEModelData::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(QVariant::fromValue(get_data()));
	}

	//--------------------------------------------------------------------------------

	void MDEModelData::data_changed()
	{
		auto editor = static_cast<ModelDataEditor*>(sender());
		int index = m_internal->m_editors.indexOf(editor);
		Q_ASSERT(index >= 0 && index < m_internal->m_data.size());
		editor->get_data(m_internal->m_data[index]);
		ModelDataEditor::data_changed();
	}



	//================================================================================
	// Bool
	//================================================================================

	struct MDEBool::Internal
	{
		QCheckBox* m_checkbox;
	};

	//--------------------------------------------------------------------------------

	MDEBool::MDEBool()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_checkbox = new QCheckBox();
		connect(m_internal->m_checkbox, &QCheckBox::toggled, this, &MDEBool::data_changed);
		layout()->addWidget(m_internal->m_checkbox);
	}

	MDEBool::~MDEBool()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEBool::set_data(const ModelDataEntry& entry)
	{
		m_internal->m_checkbox->setChecked(entry.get_value().toBool());
	}

	void MDEBool::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_checkbox->isChecked());
	}



	//================================================================================
	// Int
	//================================================================================

	struct MDEInt::Internal
	{
		QSpinBox* m_spinbox;
	};

	//--------------------------------------------------------------------------------

	MDEInt::MDEInt()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_spinbox = new QSpinBox();
		connect(m_internal->m_spinbox, (void(QSpinBox::*)(int))&QSpinBox::valueChanged, this, &MDEInt::data_changed);
		layout()->addWidget(m_internal->m_spinbox);
	}

	MDEInt::~MDEInt()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEInt::set_data(const ModelDataEntry& entry)
	{
		m_internal->m_spinbox->setMinimum(entry.get_attribute(ModelDataAttribute::MinValue, -99999).toInt());
		m_internal->m_spinbox->setMaximum(entry.get_attribute(ModelDataAttribute::MaxValue, 99999).toInt());
		m_internal->m_spinbox->setValue(entry.get_value().toInt());
	}

	void MDEInt::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_spinbox->value());
	}



	//================================================================================
	// Int (Combobox)
	//================================================================================

	struct MDEIntComboBox::Internal
	{
		QComboBox* m_combobox;
	};

	//--------------------------------------------------------------------------------

	MDEIntComboBox::MDEIntComboBox()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_combobox = new QComboBox();
		connect(m_internal->m_combobox, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &MDEIntComboBox::data_changed);
		layout()->addWidget(m_internal->m_combobox);
	}

	MDEIntComboBox::~MDEIntComboBox()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEIntComboBox::set_data(const ModelDataEntry& entry)
	{
		m_internal->m_combobox->blockSignals(true);
		m_internal->m_combobox->clear();
		m_internal->m_combobox->addItems(entry.get_attribute(ModelDataAttribute::Enum).toStringList());
		m_internal->m_combobox->blockSignals(false);
		m_internal->m_combobox->setCurrentIndex(entry.get_value().toInt());
	}

	void MDEIntComboBox::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_combobox->currentIndex());
	}
	


	//================================================================================
	// Int
	//================================================================================

	struct MDEFloat::Internal
	{
		QDoubleSpinBox* m_spinbox;
	};

	//--------------------------------------------------------------------------------

	MDEFloat::MDEFloat()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_spinbox = new QDoubleSpinBox();
		connect(m_internal->m_spinbox, (void(QDoubleSpinBox::*)(double))&QDoubleSpinBox::valueChanged, this, &MDEFloat::data_changed);
		layout()->addWidget(m_internal->m_spinbox);
	}

	MDEFloat::~MDEFloat()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEFloat::set_data(const ModelDataEntry& entry)
	{
		m_internal->m_spinbox->setMinimum(entry.get_attribute(ModelDataAttribute::MinValue, -99999.0f).toFloat());
		m_internal->m_spinbox->setMaximum(entry.get_attribute(ModelDataAttribute::MaxValue, 99999.0f).toFloat());
		m_internal->m_spinbox->setValue(entry.get_value().toDouble());
	}

	void MDEFloat::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_spinbox->value());
	}



	//================================================================================
	// String
	//================================================================================

	struct MDEString::Internal
	{
		QLineEdit* m_lineedit;
	};

	//--------------------------------------------------------------------------------

	MDEString::MDEString()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_lineedit = new QLineEdit();
		connect(m_internal->m_lineedit, &QLineEdit::editingFinished, this, &MDEString::data_changed);
		layout()->addWidget(m_internal->m_lineedit);
	}

	MDEString::~MDEString()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEString::set_data(const ModelDataEntry& entry)
	{
		m_internal->m_lineedit->setText(entry.get_value().toString());
	}

	void MDEString::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_lineedit->text());
	}



	//================================================================================
	// String (Combobox)
	//================================================================================

	struct MDEStringComboBox::Internal
	{
		QComboBox* m_combobox;
	};

	//--------------------------------------------------------------------------------

	MDEStringComboBox::MDEStringComboBox()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_combobox = new QComboBox();
		connect(m_internal->m_combobox, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &MDEStringComboBox::data_changed);
		layout()->addWidget(m_internal->m_combobox);
	}

	MDEStringComboBox::~MDEStringComboBox()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEStringComboBox::set_data(const ModelDataEntry& entry)
	{
		auto items = entry.get_attribute(ModelDataAttribute::Enum).toStringList();
		if (entry.has_attribute(ModelDataAttribute::EnumSorted))
		{
			qSort(items);
		}

		m_internal->m_combobox->blockSignals(true);
		
		m_internal->m_combobox->clear();
		m_internal->m_combobox->addItems(items);
		
		auto value = entry.get_value().toString();
		if (m_internal->m_combobox->findText(value) == -1)
		{
			m_internal->m_combobox->addItem(value);
			m_internal->m_combobox->setItemData(m_internal->m_combobox->count() - 1, QColor(255, 0, 0), Qt::ForegroundRole);
		}
		
		m_internal->m_combobox->blockSignals(false);

		m_internal->m_combobox->setCurrentText(value);
	}

	void MDEStringComboBox::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_combobox->currentText());
	}




	//================================================================================
	// String (File)
	//================================================================================

	struct MDEStringFile::Internal
	{
		QLineEdit*	 m_lineedit;
		QPushButton* m_browse_button;

		QString		 m_dir;
		QString		 m_filter;
	};

	//--------------------------------------------------------------------------------

	MDEStringFile::MDEStringFile()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_lineedit = new QLineEdit();
		m_internal->m_lineedit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
		connect(m_internal->m_lineedit, &QLineEdit::editingFinished, this, &MDEStringFile::data_changed);
		layout()->addWidget(m_internal->m_lineedit);

		m_internal->m_browse_button = new QPushButton("...");
		connect(m_internal->m_browse_button, &QPushButton::clicked, this, &MDEStringFile::slot_browse);
		layout()->addWidget(m_internal->m_browse_button);
	}

	MDEStringFile::~MDEStringFile()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEStringFile::set_data(const ModelDataEntry& entry)
	{
		m_internal->m_dir = entry.get_attribute(ModelDataAttribute::FileDir).toString();
		m_internal->m_filter = entry.get_attribute(ModelDataAttribute::FileFilter).toString();

		m_internal->m_lineedit->setText(entry.get_value().toString());
	}

	void MDEStringFile::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_lineedit->text());
	}

	//--------------------------------------------------------------------------------

	void MDEStringFile::slot_browse()
	{
		auto dir = get_absolute_path(!m_internal->m_dir.isEmpty() ? m_internal->m_dir : "Data");
		auto filter = (!m_internal->m_filter.isEmpty() ? m_internal->m_filter : "All Files (*.*)");

		auto current_filename = get_absolute_path(m_internal->m_lineedit->text());
		if (!current_filename.isEmpty())
		{
			dir = current_filename.section('/', 0, -2);
		}

		auto filename = QFileDialog::getOpenFileName(this, "Open File", dir, filter, nullptr, QFileDialog::DontResolveSymlinks);
		if (!filename.isEmpty())
		{
			m_internal->m_lineedit->setText(filename);
			data_changed();
		}
	}



	//================================================================================
	// Color
	//================================================================================

	struct MDEColor::Internal
	{
		ColorButtonWidget* m_color_button;
	};

	//--------------------------------------------------------------------------------

	MDEColor::MDEColor()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_color_button = new ColorButtonWidget();
		connect(m_internal->m_color_button, &ColorButtonWidget::signal_color_changed, this, &MDEColor::data_changed);
		layout()->addWidget(m_internal->m_color_button);
	}

	MDEColor::~MDEColor()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEColor::set_data(const ModelDataEntry& entry)
	{
		m_internal->m_color_button->set_color(entry.get_value().value<QColor>());
	}

	void MDEColor::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_color_button->get_color());
	}



	//================================================================================
	// Button
	//================================================================================

	struct MDEButton::Internal
	{
		QPushButton* m_button;
		int			 m_value;
	};

	//--------------------------------------------------------------------------------

	MDEButton::MDEButton()
		: m_internal(std::make_unique<Internal>())
	{
		m_internal->m_button = new QPushButton();
		connect(m_internal->m_button, &QPushButton::clicked, this, &MDEButton::data_changed);
		layout()->addWidget(m_internal->m_button);
	}

	MDEButton::~MDEButton()
	{
	}

	//--------------------------------------------------------------------------------

	void MDEButton::set_data(const ModelDataEntry& /*entry*/)
	{
	}

	void MDEButton::get_data(ModelDataEntry& entry) const
	{
		entry.set_value(m_internal->m_value);
	}
}