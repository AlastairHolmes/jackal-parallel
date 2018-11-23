#ifndef JKPARALLEL_MPSC_QUEUE_H
#define JKPARALLEL_MPSC_QUEUE_H

#include <jkparallel\spmc_queue.h>
#include <map>
#include <mutex>

namespace jkparallel
{

	template <class elementType, class expansionStdAllocatorType = std::allocator<elementType>>
	class mpsc_queue
	{
	public:

		using size_type = typename spmc_queue<elementType, expansionStdAllocatorType>::size_type;
		using sink_element_type = elementType;
		using source_element_type = elementType;
		using is_staged_sink = std::false_type;

		mpsc_queue(std::size_t p_producer_threads)
			: m_expected_threads(p_producer_threads),
			m_registered_threads(0)
		{}

		mpsc_queue& operator=(const mpsc_queue&) = delete;
		mpsc_queue& operator=(mpsc_queue&&) = delete;

		void register_producer(size_type p_initial_size = 1U, size_type p_maximum_size = 0U, const expansionStdAllocatorType& p_allocator = expansionStdAllocatorType())
		{
			std::unique_lock l(m_mutex);

			m_map.insert(std::make_pair(std::this_thread::get_id(), spmc_queue<elementType, expansionStdAllocatorType>(p_initial_size, p_maximum_size, p_allocator)));
			++m_registered_threads;

			if (m_registered_threads != m_expected_threads)
			{
				m_condition.wait(l, [this]()
				{
					return (m_expected_threads == m_registered_threads);
				});
			}
			else if(m_registered_threads == m_expected_threads)
			{
				m_condition.notify_all();
			}
		}

		bool try_push(sink_element_type&& p_element)
		{
			auto result = m_map.find(std::this_thread::get_id());
			return result->second.try_push(std::move(p_element));
		}

		template <class inputIteratorType>
		bool try_push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			auto result = m_map.find(std::this_thread::get_id());
			return result->second.try_push(p_begin, p_end);
		}

		template <class inputIteratorType>
		bool try_push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			auto result = m_map.find(std::this_thread::get_id());
			return result->second.try_push_move(p_begin, p_end);
		}

		void push(sink_element_type&& p_element)
		{
			auto result = m_map.find(std::this_thread::get_id());
			result->second.push(std::move(p_element));
		}

		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			auto result = m_map.find(std::this_thread::get_id());
			result->second.push(p_begin, p_end);
		}

		template <class inputIteratorType>
		void push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			auto result = m_map.find(std::this_thread::get_id());
			result->second.push_move(p_begin, p_end);
		}

		std::optional<source_element_type> try_pop()
		{
			std::optional<source_element_type> result;

			for (auto& queue : m_map)
			{
				result = queue.second.try_pop();
				if (result.has_value())
				{
					break;
				}
			}

			return result;
		}

		void flush()
		{

		}

	private:

		std::mutex m_mutex;
		std::condition_variable m_condition;
		std::size_t m_registered_threads;
		const std::size_t m_expected_threads;

		std::map<std::thread::id, spmc_queue<elementType, expansionStdAllocatorType>> m_map;

	};

}

#endif