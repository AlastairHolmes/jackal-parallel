#include <jkparallel\conditions.h>

namespace jkparallel
{

	waitable_flag::waitable_flag()
		: m_value(false)
	{
	}

	void waitable_flag::wait()
	{
		std::unique_lock<std::mutex> L(m_mutex);
		m_condition.wait(L, [this] { return m_value; });
	}

	bool waitable_flag::signalled()
	{
		std::unique_lock<std::mutex> L(m_mutex);
		return m_value;
	}

	void waitable_flag::signal()
	{
		{
			std::unique_lock<std::mutex> L(m_mutex);
			m_value = true;
		}
		m_condition.notify_all();
	}

}