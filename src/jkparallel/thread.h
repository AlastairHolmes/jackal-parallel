#ifndef JKPARALLEL_THREAD_H
#define JKPARALLEL_THREAD_H

#include <jkparallel\conditions.h>
#include <thread>
#include <optional>

namespace jkparallel
{

	class thread
	{
	public:

		thread(bool p_internal_thread = true);

		thread(const thread&) = delete;
		thread(thread&&) = delete;

		thread& operator=(const thread&) = delete;
		thread& operator=(thread&&) = delete;

		~thread();

	public:

		void prepare_external();
		static void submit_external();

		void begin(const std::function<void()>& p_function);
		void begin(std::function<void()>&& p_function);

		void join();

	private:

		static void initial_thread_function(thread* p_thread);
		static thread_local thread* this_thread;

	private:

		void set_exception();
		void rethrow_exception();

		void prepare_thread();
		void run();

	private:

		const bool m_internal_thread; //Indicates if this thread will create a thread internally, or use a externally created thread (Via prepare and submit)
		bool m_prepared; //Logic to stop multiple threads being prepared for same thread instance.

		std::optional<std::thread> m_thread;
		std::function<void()> m_function;
		std::exception_ptr m_exception;

		waitable_flag m_begin_flag;
		waitable_flag m_join_flag;

		std::mutex m_prepare_lock;
		std::mutex m_join_lock;
		std::mutex m_exception_lock;

	};

}

#endif