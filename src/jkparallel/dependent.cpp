#include <jkparallel\dependent.h>

namespace jkparallel
{

	dependent::dependent(std::size_t p_initial_dependencies)
		: m_dependencies(p_initial_dependencies)
	{
	}

	void dependent::notify(task_system_interface& p_interface)
	{
		if (m_dependencies.fetch_sub(1, std::memory_order_relaxed) == 1)
		{
			complete(p_interface);
		}
	}

	void dependent::add_dependencies(std::size_t p_dependencies)
	{
		m_dependencies.fetch_add(p_dependencies, std::memory_order_relaxed);
	}

	node::node(std::size_t p_initial_dependencies)
		: dependent(p_initial_dependencies)
	{
	}

	void node::prepend(dependent* p_dependent)
	{
		p_dependent->add_dependencies(1);
		m_notifiables.push_back(p_dependent);
	}

	void node::notify_on_end(notifiable* p_notifiable)
	{
		m_notifiables.push_back(p_notifiable);
	}

	void node::end(task_system_interface& p_interface)
	{
		for (auto x : m_notifiables)
		{
			x->notify(p_interface);
		}
	}

}