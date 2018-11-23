#ifndef JKPARALLEL_ASYNC_H
#define JKPARALLEL_ASYNC_H

#include <jkutil\array_proxy.h>
#include <jkutil\allocator.h>
#include <jkutil\memory.h>
#include <memory>
#include <optional>

namespace jkparallel
{

	class base_callable
	{
	public:

		virtual void operator()() = 0;

	};

	class base_callback
	{
	public:

		base_callback() = default;

		virtual ~base_callback() = default;

		virtual void callback() = 0;

		void set_memory(const jkutil::array_proxy<char>& p_memory)
		{
			m_memory = p_memory;
		}

		jkutil::array_proxy<char> get_memory() const
		{
			return m_memory;
		}

	private:

		jkutil::array_proxy<char> m_memory;

	};

	template <class operationCallableType, class callbackOperationType, class exceptionHandlerType, class callbackSinkType>
	class async_task : public base_callable, public base_callback
	{
	public:

		static_assert(std::is_same_v<typename callbackSinkType::sink_element_type, base_callback*>);

		async_task(operationCallableType&& p_callable, callbackOperationType&& p_callback, exceptionHandlerType&& p_exception_handler, callbackSinkType& p_callback_sink)
			: m_callable(std::move(p_callable)),
			m_callback(std::move(p_callback)),
			m_exception_handler(std::move(p_exception_handler)),
			m_sink(p_callback_sink),
			m_result(),
			m_exception(nullptr)
		{

		}

		virtual void operator()() override final
		{
			try
			{
				if constexpr (is_void)
				{
					m_callable();
				}
				else
				{
					m_result.emplace(m_callable());
				}
			}
			catch (...)
			{
				m_exception = std::current_exception();
			}
			m_sink.push(this);
		}

		virtual void callback() override final
		{
			if (m_exception != nullptr || (!m_result.has_value() && !is_void))
			{
				m_exception_handler(m_exception);
			}
			else
			{
				if constexpr (is_void)
				{
					m_callback();
				}
				else
				{
					m_callback(m_result.value());
				}
			}
		}

	private:

		operationCallableType m_callable;

		static const bool is_void = std::is_same_v<decltype(m_callable()), void>;

		callbackOperationType m_callback;
		exceptionHandlerType m_exception_handler;
		callbackSinkType& m_sink;
		std::optional<std::conditional_t<is_void, bool, decltype(m_callable())>> m_result;
		std::exception_ptr m_exception;

	};

	template <class sinkType, class callbackQueueType, class allocatorType = jkutil::allocator>
	class async
	{
	public:

		//Note: Possible memory leak. If the sink never runs the submitted task, or if the callback of a submitted task is never run.
			//Should try to fix this.
			//As minimum this case should be detected and throw an error.

		static_assert(std::is_same_v<typename sinkType::sink_element_type, base_callable*>);
		static_assert(std::is_same_v<typename callbackQueueType::sink_element_type, base_callback*>);
		static_assert(std::is_same_v<typename callbackQueueType::source_element_type, base_callback*>);

		async(const sinkType& p_sink, const callbackQueueType& p_callback_sink, const allocatorType& p_allocator = allocatorType())
			: m_allocator(p_allocator),
			m_sink(p_sink),
			m_callback_queue(p_callback_sink)
		{}

		async(sinkType&& p_sink, callbackQueueType&& p_callback_sink, const allocatorType& p_allocator = allocatorType())
			: m_allocator(p_allocator),
			m_sink(std::move(p_sink)),
			m_callback_queue(std::move(p_callback_sink))
		{}

		async& operator=(const async&) = delete;
		async& operator=(async&&) = delete;

		template <class callableType, class callbackType, class exceptionHandlerType> 
		void submit(callableType&& p_operation, callbackType&& p_callback, exceptionHandlerType&& p_exception_handler)
		{
			using created_type = async_task<callableType, callbackType, exceptionHandlerType, callbackQueueType>;

			created_type* ptr = jkutil::create<created_type>(m_allocator,
				callableType(std::forward<callableType>(p_operation)),
				callbackType(std::forward<callbackType>(p_callback)),
				exceptionHandlerType(std::forward<exceptionHandlerType>(p_exception_handler)),
				m_callback_queue);

			ptr->set_memory(jkutil::make_array_proxy(sizeof(created_type),reinterpret_cast<char*>(ptr)));

			m_sink.push(static_cast<base_callable*>(ptr));
		}

		void process_callbacks()
		{
			bool none = false;
			while (!none)
			{
				std::optional<base_callback*> callback = m_callback_queue.try_pop();
				if (callback.has_value())
				{
					base_callback* ptr = callback.value();

					auto memory = ptr->get_memory();
					auto guard = jkutil::make_deallocate_guard(m_allocator, memory.data(), memory.size());

					ptr->callback();
					jkutil::destruct(ptr);
				}
				else
				{
					none = true;
				}
			}
		}

	private:

		allocatorType m_allocator;
		sinkType m_sink;
		callbackQueueType m_callback_queue;

	};

}

#endif