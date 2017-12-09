// Project includes
#include "Utility/ModelData/ModelDataDelegate.h"
#include "Utility/ModelData/ModelData.h"
#include "Utility/ModelData/ModelDataEditor.h"


namespace Utility
{
	//================================================================================
	// Internal
	//================================================================================

	struct ModelDataDelegate::Internal
	{
		mutable QMap<QPersistentModelIndex, MDEModelData*> m_editors;
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	ModelDataDelegate::ModelDataDelegate(QObject* parent)
		: QStyledItemDelegate(parent)
		, m_internal(std::make_unique<Internal>())
	{
	}

	ModelDataDelegate::~ModelDataDelegate()
	{
	}



	//================================================================================
	// QAbstractItemDelegate Interface
	//================================================================================
	
	QWidget* ModelDataDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		auto data = index.data(ModelDataRole);
		if (data.isValid())
		{
			auto editor = new MDEModelData();
			editor->setParent(parent);
			editor->set_data(data.value<ModelData>());
			connect(editor, &MDEModelData::signal_data_changed, this, &ModelDataDelegate::slot_data_changed);
			connect(editor, &MDEModelData::destroyed, this, &ModelDataDelegate::slot_editor_destroyed);
			m_internal->m_editors.insert(index, editor);
			const_cast<ModelDataDelegate*>(this)->sizeHintChanged(index);
			return editor;
		}
		else
		{
			return QStyledItemDelegate::createEditor(parent, option, index);
		}
	}

	void ModelDataDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
	{
		if (m_internal->m_editors.contains(index))
		{
			auto data = index.data(ModelDataRole).value<ModelData>();
			auto md_editor = static_cast<MDEModelData*>(editor);
			md_editor->set_data(data);
		}
		else
		{
			QStyledItemDelegate::setEditorData(editor, index);
		}
	}

	void ModelDataDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		if (m_internal->m_editors.contains(index))
		{
			auto md_editor = static_cast<MDEModelData*>(editor);
			
			auto& new_data = md_editor->get_data();
			auto old_data = model->data(index, ModelDataRole).value<ModelData>();

			if (new_data != old_data)
			{
				model->setData(index, QVariant::fromValue(new_data), ModelDataRole);

				md_editor->blockSignals(true);
				md_editor->set_data(model->data(index, ModelDataRole).value<ModelData>());
				md_editor->blockSignals(false);
			}
		}
		else
		{
			QStyledItemDelegate::setModelData(editor, model, index);
		}
	}

	QSize ModelDataDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		auto it = m_internal->m_editors.find(index);
		return (it != m_internal->m_editors.end() ? (*it)->sizeHint() : QStyledItemDelegate::sizeHint(option, index));
	}

	void ModelDataDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
	{
		QStyledItemDelegate::initStyleOption(option, index);

		auto display_value = index.data(Qt::DisplayRole);
		if (!display_value.isValid())
		{
			auto value = index.data(ModelDataRole);
			if (value.isValid() && !value.isNull())
			{
				auto data = value.value<ModelData>();
				if (data.size() == 1)
				{
					auto entry_value = data[0].get_value();
					if (entry_value.canConvert<QString>())
					{
						option->features |= QStyleOptionViewItem::HasDisplay;
						option->text = displayText(entry_value.toString(), option->locale);
					}
				}
			}
		}
	}



	//================================================================================
	// Editor Slots
	//================================================================================

	void ModelDataDelegate::slot_data_changed()
	{
		auto editor = static_cast<MDEModelData*>(sender());
		emit commitData(editor);
	}

	void ModelDataDelegate::slot_editor_destroyed()
	{
		auto object = sender();
		
		auto it = std::find_if(m_internal->m_editors.begin(), m_internal->m_editors.end(), [object] (MDEModelData* editor)
		{
			return (editor == object);
		});

		emit sizeHintChanged(it.key());

		m_internal->m_editors.erase(it);
	}
}