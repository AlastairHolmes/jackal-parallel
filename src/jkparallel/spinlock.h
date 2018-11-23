#ifndef JKPARALLEL_SPINLOCK_H
#define JKPARALLEL_SPINLOCK_H

#include <atomic>

namespace jkparallel
{

	class spinlock
	{
	public:

		spinlock();

		bool try_lock() noexcept;

		void lock() noexcept;

		void unlock() noexcept;

	private:

		std::atomic_flag m_flag;

	};

}

#endif