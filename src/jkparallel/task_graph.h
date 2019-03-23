#ifndef JKPARALLEL_TASK_GRAPH_H
#define JKPARALLEL_TASK_GRAPH_H

#include <jkparallel\dependent.h>
#include <jkutil\memory.h>
#include <jkutil\derived.h>
#include <jkutil\transform_iterator.h>
#include <jkutil\selective_iterator.h>

namespace jkparallel
{

	class node_task : public task, public node
	{
	public:

		node_task(std::size_t p_initial_dependencies);

		node_task(const node_task&) = delete;
		node_task(node_task&& p_instance);

		node_task& operator=(const node_task&) = delete;
		node_task& operator=(node_task&&) = delete;

		void set_self_reference(jkutil::derived<node_task>&& p_self_reference);
		void destroy();

	protected:

		virtual void operation(task_system_interface& p_interface, dependent& p_end_dependent) = 0;

	private:

		class end_dependent : public dependent
		{
		public:

			end_dependent(node_task& p_task, std::size_t p_dependencies);

			virtual void complete(task_system_interface& p_interface) override final;

		private:

			node_task& m_task;

		};

		friend class end_dependent;

	private:

		//All Dependencies have notified the node
		virtual void complete(task_system_interface& p_interface) override final;

		//The task system is running this task
		virtual void operator()(task_system_interface& p_interface) override final;

		end_dependent m_end_dependent;
		jkutil::derived<node_task> m_self_reference;

	};

	template <class callableType>
	class callable_node_task : public node_task
	{
	public:

		callable_node_task(std::size_t p_initial_dependencies, callableType p_callable)
			: node_task(p_initial_dependencies), m_callable(p_callable)
		{}

	protected:

		virtual void operation(task_system_interface& p_interface, dependent& p_end_dependent)
		{
			m_callable(p_interface, p_end_dependent);
		}

	private:

		callableType m_callable;

	};

	class task_graph
	{
	public:

		task_graph() = default;

		template <class callableType>
		node* create_task(callableType&& p_callable)
		{
			jkutil::derived<node_task> x;
			x.emplace<callable_node_task<std::decay_t<callableType>>>(1, std::forward<callableType>(p_callable));
			node_task* temp = x.get();
			m_tasks.emplace_back(std::make_pair(std::move(x), temp));
			return temp;
		}

		void staged_submit(task_system_interface& p_interface)
		{
			for (auto& x : m_tasks)
			{
				x.first.get()->set_self_reference(std::move(x.first));
				x.second->notify(p_interface);
				x.second = nullptr;
			}

			m_tasks.clear();
		}

	private:

		std::vector<std::pair<jkutil::derived<node_task>, node_task*>> m_tasks;

	};

}

#endif