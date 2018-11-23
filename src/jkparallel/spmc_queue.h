#ifndef JKPARALLEL_SPMC_QUEUE_H
#define JKPARALLEL_SPMC_QUEUE_H

#include <jkparallel/queue.h>

namespace jkparallel
{

	template <class elementType, class expansionStdAllocatorType = std::allocator<elementType>>
	class spmc_queue
	{
	private:

		using queue_type = queue_interface<elementType, expansionStdAllocatorType>;

	public:

		using size_type = typename queue_type::queue_index_t;
		using sink_element_type = elementType;
		using source_element_type = elementType;
		using is_staged_sink = std::false_type;

		spmc_queue(size_type p_initial_size = 1U, size_type p_maximum_size = 0U, const expansionStdAllocatorType& p_allocator = expansionStdAllocatorType())
			: m_queue(p_initial_size, p_maximum_size, p_allocator)
		{}

		bool try_push(sink_element_type&& p_element)
		{
			return m_queue.try_push(std::move(p_element));
		}

		template <class inputIteratorType>
		bool try_push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			return m_queue.try_push(p_begin, p_end);
		}

		template <class inputIteratorType>
		bool try_push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			return m_queue.try_push_move(p_begin, p_end);
		}

		void push(sink_element_type&& p_element)
		{
			m_queue.push(std::move(p_element));
		}

		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_queue.push(p_begin, p_end);
		}

		template <class inputIteratorType>
		void push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_queue.push_move(p_begin, p_end);
		}

		std::optional<source_element_type> try_pop()
		{
			return m_queue.steal();
		}

		void flush()
		{
			m_queue.flush();
		}

	private:

		queue_interface<elementType, expansionStdAllocatorType> m_queue;

	};

}

#endif