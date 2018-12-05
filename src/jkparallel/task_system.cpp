#include <jkparallel\task_system.h>

namespace jkparallel
{

	task_system_interface::task_system_interface(const queue_interface_reference<task*>& p_interface_reference)
		: m_interface(queue_interface_reference<task*>(p_interface_reference))
	{
	}

	bool task_system_interface::try_push(task*&& p_element)
	{
		return m_interface.try_push(std::move(p_element));
	}

	void task_system_interface::push(task*&& p_element)
	{
		m_interface.push(std::move(p_element));
	}

	void task_system_interface::try_run_task()
	{
		auto task = m_interface.get_internal_sink().try_pop();
		if (task.has_value())
		{
			task.value()->operator()(*this);
		}
	}

	void task_system_interface::flush()
	{
		m_interface.get_internal_sink().flush();
	}

	task_system::task_system()
		: m_queue(16, 0)
	{
	}

	task_system_interface task_system::create_interface()
	{
		return m_queue.create_interface();
	}

	void task_system::finalize_interfaces()
	{
		m_queue.finalize_interfaces();
	}

}