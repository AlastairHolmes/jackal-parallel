#ifndef JKPARALLEL_TASK_SYSTEM_H
#define JKPARALLEL_TASK_SYSTEM_H

#include <jkparallel\queue.h>
#include <jkparallel\sinks.h>
#include <vector>

namespace jkparallel
{

	class task;

	class task_system_interface : public simple_sink_stager<queue_interface_reference<task*>>
	{
	public:

		using sink_element_type = task*;
		using is_staged_sink = std::false_type;

		task_system_interface(const queue_interface_reference<task*>& p_interface_reference);

		task_system_interface(const task_system_interface&) = delete;
		task_system_interface(task_system_interface&&) = delete;

		task_system_interface& operator=(const task_system_interface&) = delete;
		task_system_interface& operator=(task_system_interface&&) = delete;

		void try_run_task();
		void flush();

	};

	class task
	{
	public:

		virtual void operator()(task_system_interface& p_interface) = 0;

	};

	class task_system
	{
	public:

		task_system();

		[[nodiscard]] task_system_interface create_interface();
		void finalize_interfaces();

	private:

		queue<task*> m_queue;

	};

}

#endif