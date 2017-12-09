// Project includes
#include "Utility/WidgetState/WidgetStateManager.h"
#include "Utility/WidgetState/WidgetState.h"

// Qt includes
#include <QDataStream>
#include <QEvent>
#include <QHash>
#include <QSettings>
#include <QVariant>


namespace Utility
{
	//================================================================================
	// Internal
	//================================================================================

	struct WidgetStateManager::Internal
	{
		struct Entry
		{
			QWidget* m_widget;
			bool	 m_state_restored;

			Entry()
				: m_widget(nullptr)
				, m_state_restored(false)
			{
			}

			Entry(QWidget& widget)
				: m_widget(&widget)
				, m_state_restored(false)
			{
			}
		};

		QVector<Entry>					m_entries;
		QHash<QString, QByteArray>		m_states;
		QVector<WidgetStateDelegatePtr>	m_delegates;
	};



	//================================================================================
	// Construction & Destruction
	//================================================================================

	WidgetStateManager::WidgetStateManager(QObject* parent)
		: QObject(parent)
		, m_internal(std::make_unique<Internal>())
	{
	}

	WidgetStateManager::~WidgetStateManager()
	{
	}



	//================================================================================
	// Registration
	//================================================================================

	void WidgetStateManager::register_widget(QWidget& widget)
	{
		widget.installEventFilter(this);

		auto name = widget.objectName();
		if (!name.isEmpty())
		{
			auto it = std::find_if(m_internal->m_entries.begin(), m_internal->m_entries.end(), [&widget] (const Internal::Entry& entry)
			{
				return (entry.m_widget == &widget || entry.m_widget->objectName() == widget.objectName());
			});
			if (it == m_internal->m_entries.end())
			{
				m_internal->m_entries << Internal::Entry(widget);
			}
		}

		for (auto child : widget.children())
		{
			if (child->isWidgetType())
			{
				register_widget(static_cast<QWidget&>(*child));
			}
		}
	}



	//================================================================================
	// Settings
	//================================================================================

	void WidgetStateManager::load(const QSettings& settings)
	{
		m_internal->m_states.clear();

		auto data = settings.value("WidgetStates").toMap();
		for (auto it = data.begin(); it != data.end(); ++it)
		{
			m_internal->m_states.insert(it.key(), it->toByteArray());
		}
	}

	void WidgetStateManager::save(QSettings& settings)
	{
		QVariantMap data;
		for (auto it = m_internal->m_states.begin(); it != m_internal->m_states.end(); ++it)
		{
			data.insert(it.key(), *it);
		}
		settings.setValue("WidgetStates", data);
	}



	//================================================================================
	// Delegate
	//================================================================================

	void WidgetStateManager::add_delegate(WidgetStateDelegatePtr state_delegate)
	{
		Q_ASSERT(!m_internal->m_delegates.contains(state_delegate));
		m_internal->m_delegates << state_delegate;
	}

	void WidgetStateManager::remove_delegate(WidgetStateDelegatePtr state_delegate)
	{
		Q_ASSERT(m_internal->m_delegates.contains(state_delegate));
		m_internal->m_delegates.removeOne(state_delegate);
	}



	//================================================================================
	// Qt Events
	//================================================================================

	bool WidgetStateManager::eventFilter(QObject* object, QEvent* event)
	{
		auto it = std::find_if(m_internal->m_entries.begin(), m_internal->m_entries.end(), [object] (const Internal::Entry& entry)
		{
			return (entry.m_widget == object);
		});

		if (event->type() == QEvent::Show)
		{
			if (it != m_internal->m_entries.end() && !it->m_state_restored)
			{
				load_state(static_cast<QWidget&>(*object));
				it->m_state_restored = true;
			}
		}

		if (event->type() == QEvent::Hide)
		{
			if (it != m_internal->m_entries.end())
			{
				save_state(static_cast<QWidget&>(*object));
			}
		}

		if (event->type() == QEvent::Destroy)
		{
			object->removeEventFilter(this);

			if (it != m_internal->m_entries.end())
			{
				m_internal->m_entries.erase(it);
			}
		}

		if (event->type() == QEvent::ChildAdded)
		{
			auto child_event = static_cast<QChildEvent*>(event);
			if (child_event->child()->isWidgetType())
			{
				register_widget(static_cast<QWidget&>(*child_event->child()));
			}
		}

		return false;
	}



	//================================================================================
	// State
	//================================================================================

	void WidgetStateManager::load_state(QWidget& widget)
	{
		auto data_it = m_internal->m_states.find(widget.objectName());
		if (data_it != m_internal->m_states.end())
		{
			QDataStream stream(*data_it);

			load_widget_state(widget, stream);

			for (auto state_delegate : m_internal->m_delegates)
			{
				state_delegate->on_load_state(widget);
			}
		}
	}

	void WidgetStateManager::save_state(QWidget& widget)
	{
		QByteArray data;
		QDataStream stream(&data, QIODevice::WriteOnly);

		save_widget_state(widget, stream);

		for (auto state_delegate : m_internal->m_delegates)
		{
			state_delegate->on_save_state(widget);
		}

		m_internal->m_states.insert(widget.objectName(), data);
	}
}