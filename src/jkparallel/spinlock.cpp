#include <jkparallel\spinlock.h>

namespace jkparallel
{

	spinlock::spinlock()
		: m_flag()
	{

	}

	bool spinlock::try_lock() noexcept
	{
		return !m_flag.test_and_set(std::memory_order_acquire);
	}

	void spinlock::lock() noexcept
	{
		while (m_flag.test_and_set(std::memory_order_acquire));
	}

	void spinlock::unlock() noexcept
	{
		m_flag.clear(std::memory_order_release);
	}

}