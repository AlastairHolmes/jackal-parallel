#ifndef JKPARALLEL_TASK_PROCESSOR_H
#define JKPARALLEL_TASK_PROCESSOR_H

#include <jkparallel\thread.h>

namespace jkparallel
{

	class task_processor
	{
	public:

		task_processor(bool p_internal_thread = true);

		void begin();
		void pause();

		void end();
		void join();

	private:

		void unpause();

		thread m_thread;

	};

}

#endif