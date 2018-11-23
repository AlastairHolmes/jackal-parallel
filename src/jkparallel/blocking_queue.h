#ifndef JKPARALLEL_BLOCKING_QUEUE_H
#define JKPARALLEL_BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <optional>
#include <jkutil\utility.h>

namespace jkparallel
{

	template <class elementType, class stdAllocatorType = std::allocator<elementType>>
	class blocking_queue
	{
	public:

		using sink_element_type = elementType;
		using source_element_type = elementType;
		using is_staged_sink = std::false_type;

		blocking_queue(const stdAllocatorType& p_allocator = stdAllocatorType())
			: m_mutex(), m_queue(std::deque<elementType, stdAllocatorType>(p_allocator))
		{
			
		}

		blocking_queue(const blocking_queue& p_instance)
			: m_queue(p_instance.m_queue)
		{
		}

		blocking_queue(blocking_queue&& p_instance)
			: m_queue(p_instance.m_queue)
		{
		}

		bool try_push(sink_element_type&& p_element)
		{
			push(std::move(p_element));
			return true;
		}

		template <class inputIteratorType>
		bool try_push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			push(p_begin, p_end);
			return true;
		}

		template <class inputIteratorType>
		bool try_push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			push_move(p_begin, p_end);
			return true;
		}

		void push(sink_element_type&& p_element)
		{
			std::scoped_lock<std::mutex> guard(m_mutex);
			m_queue.push(std::move(p_element));
		}

		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			std::scoped_lock<std::mutex> guard(m_mutex);
			for (const auto& i : jkutil::make_iterator_range(p_begin, p_end))
			{
				m_queue.push(i);
			}
		}

		template <class inputIteratorType>
		void push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			std::scoped_lock<std::mutex> guard(m_mutex);
			for (auto& i : jkutil::make_iterator_range(p_begin, p_end))
			{
				m_queue.push(std::move(i));
			}
		}

		std::optional<source_element_type> try_pop()
		{
			std::scoped_lock<std::mutex> guard(m_mutex);

			std::optional<source_element_type> result;

			if (!m_queue.empty())
			{
				result.emplace(std::move(m_queue.back()));
				m_queue.pop();
			}

			return result;
		}

		void flush()
		{}

	private:

		std::mutex m_mutex;
		std::queue<elementType, std::deque<elementType, stdAllocatorType>> m_queue;

	};

}

#endif