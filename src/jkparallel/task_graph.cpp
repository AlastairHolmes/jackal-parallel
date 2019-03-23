#include <jkparallel\task_graph.h>

namespace jkparallel
{

	node_task::node_task(std::size_t p_initial_dependencies)
		: node(p_initial_dependencies),
		m_end_dependent(*this, 1)
	{}

	node_task::node_task(node_task&& p_instance)
		: node(0),
		m_end_dependent(*this, 1)
	{
		throw;
	}

	void node_task::set_self_reference(jkutil::derived<node_task>&& p_self_reference)
	{
		m_self_reference = std::move(p_self_reference);
	}

	void node_task::destroy()
	{
		m_self_reference.reset();
	}

	node_task::end_dependent::end_dependent(node_task& p_task, std::size_t p_dependencies)
		: dependent(p_dependencies),
		m_task(p_task)
	{
	}

	void node_task::end_dependent::complete(task_system_interface& p_interface)
	{
		m_task.end(p_interface);
		m_task.destroy();
	}

	void node_task::complete(task_system_interface& p_interface)
	{
		p_interface.stage_push(this);
	}

	void node_task::operator()(task_system_interface& p_interface)
	{
		operation(p_interface, m_end_dependent);
		m_end_dependent.notify(p_interface);
	}

}