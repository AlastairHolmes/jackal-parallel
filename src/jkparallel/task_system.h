#ifndef JKPARALLEL_TASK_SYSTEM_H
#define JKPARALLEL_TASK_SYSTEM_H

#include <jkparallel\queue.h>
#include <jkparallel\sinks.h>
#include <vector>

namespace jkparallel
{

	class task;

	class task_system_interface
	{
	public:

		using sink_element_type = task*;
		using is_staged_sink = std::false_type;

		task_system_interface(const queue_interface_reference<task*>& p_interface_reference);

		task_system_interface(const task_system_interface&) = delete;
		task_system_interface(task_system_interface&&) = delete;

		task_system_interface& operator=(const task_system_interface&) = delete;
		task_system_interface& operator=(task_system_interface&&) = delete;

		bool try_push(task*&& p_element);
		template <class inputIteratorType>
		bool try_push(inputIteratorType p_begin, inputIteratorType p_end);
		template <class inputIteratorType>
		bool try_push_move(inputIteratorType p_begin, inputIteratorType p_end);

		void push(task*&& p_element);
		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end);
		template <class inputIteratorType>
		void push_move(inputIteratorType p_begin, inputIteratorType p_end);

		void try_run_task();
		void flush();

	private:

		simple_sink_stager<queue_interface_reference<task*>> m_interface;

	};

	template<class inputIteratorType>
	inline bool task_system_interface::try_push(inputIteratorType p_begin, inputIteratorType p_end)
	{
		return m_interface.try_push(p_begin, p_end);
	}

	template<class inputIteratorType>
	inline bool task_system_interface::try_push_move(inputIteratorType p_begin, inputIteratorType p_end)
	{
		return m_interface.try_push_move(p_begin, p_end);
	}

	template<class inputIteratorType>
	inline void task_system_interface::push(inputIteratorType p_begin, inputIteratorType p_end)
	{
		m_interface.push(p_begin, p_end);
	}

	template<class inputIteratorType>
	inline void task_system_interface::push_move(inputIteratorType p_begin, inputIteratorType p_end)
	{
		m_interface.push_move(p_begin, p_end);
	}

	class task
	{
	public:

		virtual void operator()(task_system_interface& p_interface) = 0;

	};

	class task_system
	{
	public:

		task_system();

		task_system_interface create_interface();
		void finalize_interfaces();

	private:

		queue<task*> m_queue;

	};

}

#endif