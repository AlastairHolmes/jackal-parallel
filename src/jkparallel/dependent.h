#ifndef JKPARALLEL_DEPENDENT_H
#define JKPARALLEL_DEPENDENT_H

#include <jkparallel\task_system.h>
#include <jkutil\memory.h>
#include <vector>

namespace jkparallel
{

	class notifiable
	{
	public:

		virtual void notify(task_system_interface& p_interface) = 0;

	};

	class dependent : public notifiable
	{
	public:

		dependent(std::size_t p_initial_dependencies);

		virtual void notify(task_system_interface& p_interface) override final;

		void add_dependencies(std::size_t p_dependencies);

	protected:

		virtual void complete(task_system_interface& p_interface) = 0;

	private:

		std::atomic<std::size_t> m_dependencies;

	};

	class node : public dependent
	{
	public:

		node(std::size_t p_initial_dependencies);

		void prepend(dependent* p_dependent);
		void notify_on_end(notifiable* p_notifiable);

	protected:

		void end(task_system_interface& p_interface);

	private:

		std::vector<notifiable*> m_notifiables;

	};

}

#endif