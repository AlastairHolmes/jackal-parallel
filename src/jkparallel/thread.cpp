#include <jkparallel\thread.h>
#include <jkutil\assert.h>
#include <jkutil\scoped_guard.h>

namespace jkparallel
{

	thread_local thread* thread::this_thread = nullptr;

	thread::thread(bool p_internal_thread)
		: m_internal_thread(p_internal_thread), m_prepared(false)
	{
	}

	thread::~thread()
	{
	}

	void thread::prepare_external()
	{
		std::scoped_lock<std::mutex> L(m_prepare_lock); //stop multiple threads (This ensures any errors will be detected).

		if (m_prepared)
		{
			throw std::logic_error("Cannot use a jkparallel::thread instance to prepare more than one external thread.");
		}

		if (m_internal_thread)
		{
			throw std::logic_error("Cannot use a jkparallel::thread instance constructed as a 'internal thread' to prepare an external thread.");
		}

		if (this_thread)
		{
			throw std::logic_error("Cannot prepare a external thread multiple times, without an intervening 'submit_external' call.");
		}

		this_thread = this;
		m_prepared = true;
	}

	void thread::submit_external() 
	{
		auto deprepare_guard = jkutil::make_scoped_guard([]()
		{
			this_thread = nullptr;
		});

		if (this_thread && this_thread->m_internal_thread)
		{
			throw std::logic_error("Must prepare an external thread before call to 'submit_external'.");
		}

		initial_thread_function(this_thread);
	}

	void thread::begin(const std::function<void()>& p_function)
	{
		auto guard = jkutil::make_scoped_guard([this]()
		{
			set_exception();
			m_join_flag.signal();
		});

		auto begin_guard = jkutil::make_scoped_guard([this]()
		{
			//After thread creation to ensure in join once m_begin_flag.wait(); m_thread will have a value.
			m_begin_flag.signal();
		});

		m_function = p_function;

		if (m_internal_thread)
		{
			m_thread.emplace(initial_thread_function, this);
		}

		guard.disable();
	}

	void thread::begin(std::function<void()>&& p_function)
	{
		auto guard = jkutil::make_scoped_guard([this]()
		{
			set_exception();
			m_join_flag.signal();
		});

		auto begin_guard = jkutil::make_scoped_guard([this]()
		{
			//After thread creation to ensure in join once m_begin_flag.wait(); m_thread will have a value.
			m_begin_flag.signal();
		});

		m_function = std::move(p_function);

		if (m_internal_thread)
		{
			m_thread.emplace(initial_thread_function,this);
		}

		guard.disable();
	}

	void thread::join()
	{
		if (this_thread == this)
		{
			throw std::logic_error("Cannot join self.");
		}

		m_begin_flag.wait();
		if (m_internal_thread)
		{
			std::scoped_lock<std::mutex> L(m_join_lock); //stop multiple threads from trying to join().
			if (m_thread.value().joinable())
			{
				m_thread.value().join();
			}
		}
		m_join_flag.wait();
		rethrow_exception();
	}

	void thread::initial_thread_function(thread* p_thread)
	{
		JKUTIL_ASSERT(p_thread != nullptr);
		p_thread->run();
	}

	void thread::set_exception()
	{
		std::scoped_lock<std::mutex> L(m_exception_lock);
		if (!m_exception)
		{
			m_exception = std::current_exception();
		}
	}

	void thread::rethrow_exception()
	{
		std::exception_ptr except_ptr;
		{
			std::scoped_lock<std::mutex> L(m_exception_lock);
			except_ptr = m_exception;
		}
		if (except_ptr)
		{
			std::rethrow_exception(except_ptr);
		}
	}

	void thread::prepare_thread()
	{
		this_thread = this;
	}

	void thread::run()
	{
		auto join_guard = jkutil::make_scoped_guard([this]()
		{
			m_join_flag.signal();
		});

		try
		{
			m_begin_flag.wait();
			prepare_thread();
			m_function();
		}
		catch (...)
		{
			set_exception();
		}
	}

}