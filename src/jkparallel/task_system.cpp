#include <jkparallel\task_system.h>
#include <jkutil\scoped_guard.h>

namespace jkparallel
{

	task_system_interface::task_system_interface(const queue_interface_reference<task*>& p_interface_reference)
		: simple_sink_stager<queue_interface_reference<task*>>(queue_interface_reference<task*>(p_interface_reference))
	{
	}

	void task_system_interface::try_run_task()
	{
		auto task = get_internal_sink().try_pop();
		if (task.has_value())
		{
			jkutil::make_scoped_guard([this]()
			{
				finalize_push();
			});
			task.value()->operator()(*this);
		}
	}

	void task_system_interface::flush()
	{
		get_internal_sink().flush();
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