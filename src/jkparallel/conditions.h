#ifndef JKPARALLEL_CONDITIONS_H
#define JKPARALLEL_CONDITIONS_H

#include <mutex>
#include <condition_variable>

namespace jkparallel
{

	class waitable_flag
	{
	public:

		waitable_flag();

		void wait();

		bool signalled();

		void signal();

	private:

		std::mutex m_mutex;
		std::condition_variable m_condition;
		bool m_value;

	};

}

#endif