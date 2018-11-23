#ifndef JKPARALLEL_QUEUE_H
#define JKPARALLEL_QUEUE_H

#include <jkparallel\queues.h>
#include <jkutil\assert.h>
#include <jkutil\bitwise.h>
#include <optional>
#include <type_traits>
#include <list>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <atomic>

namespace jkparallel
{

	template <class elementType, class expansionStdAllocatorType = std::allocator<elementType>>
	class queue_interface
	{
	public:

		static_assert(std::is_trivially_copyable_v<elementType>);

		using queue_index_t = uint32_t;
		using sink_element_type = elementType;
		using source_element_type = elementType;
		using is_staged_sink = std::false_type;

		queue_interface(queue_index_t p_initial_size = 1U, queue_index_t p_maximum_size = 0U, const expansionStdAllocatorType& p_allocator = expansionStdAllocatorType());

		queue_interface(const queue_interface& p_instance)
			: m_allocator(p_instance.m_allocator),
			m_buffers(p_instance.m_buffers),
			m_current_buffer(nullptr),
			m_size(p_instance.m_size),
			m_mask(m_size - 1),
			m_maximum_size(p_instance.m_maximum_size),
			m_pop_possible(false),
			m_atomic_top(0),
			m_atomic_bottom(0),
			m_local_bottom(0),
			m_steal_sources(p_instance.m_steal_sources),
			m_next_steal_source(m_steal_sources.begin())
		{
			if (m_buffers.capacity() < std::numeric_limits<queue_index_t>::digits)
			{
				m_buffers.reserve(std::numeric_limits<queue_index_t>::digits - m_buffers.capacity());
			}
			std::vector<elementType, expansionStdAllocatorType>& buffer = m_buffers.back();
			m_current_buffer = buffer.data();
		}

		queue_interface(queue_interface&& p_instance)
			: m_allocator(p_instance.m_allocator),
			m_buffers(p_instance.m_buffers),
			m_current_buffer(nullptr),
			m_size(p_instance.m_size),
			m_mask(m_size - 1),
			m_maximum_size(p_instance.m_maximum_size),
			m_pop_possible(false),
			m_atomic_top(0),
			m_atomic_bottom(0),
			m_local_bottom(0),
			m_steal_sources(p_instance.m_steal_sources),
			m_next_steal_source(m_steal_sources.begin())
		{
			if (m_buffers.capacity() < std::numeric_limits<queue_index_t>::digits)
			{
				m_buffers.reserve(std::numeric_limits<queue_index_t>::digits - m_buffers.capacity());
			}
			std::vector<elementType, expansionStdAllocatorType>& buffer = m_buffers.back();
			m_current_buffer = buffer.data();
		}

		queue_interface& operator=(const queue_interface&) = delete;
		queue_interface& operator=(queue_interface&&) = delete;

		void add_stealing_source(queue_interface& p_source);
		void finalize_stealing_sources();

		bool try_push(sink_element_type&& p_element);
		template <class inputIteratorType>
		bool try_push(inputIteratorType p_begin, inputIteratorType p_end);
		template <class inputIteratorType>
		bool try_push_move(inputIteratorType p_begin, inputIteratorType p_end);

		void push(sink_element_type&& p_element);
		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end);
		template <class inputIteratorType>
		void push_move(inputIteratorType p_begin, inputIteratorType p_end);

		std::optional<source_element_type> try_pop();
		void flush();

	public:

		std::optional<source_element_type> steal();

	protected:

		void push_operation(sink_element_type&& p_element);
		template <class inputIteratorType>
		void push_operation(inputIteratorType p_begin, inputIteratorType p_end, size_t p_distance);

		std::optional<source_element_type> try_steal();
		std::optional<source_element_type> pop_operation();

		void expand(queue_index_t p_size);
		void expand(queue_index_t p_size, elementType&& p_pushed_element);
		template <class inputIteratorType>
		void expand(queue_index_t p_size, inputIteratorType p_begin, inputIteratorType p_end, size_t p_distance);
		void expand_operation(queue_index_t p_size);

		void copy_buffer(elementType* p_destination, queue_index_t p_src_begin, queue_index_t p_src_end);

	private:

		queue_index_t remaining_space();
		typename std::vector<queue_interface*>::iterator next_steal_source();

	private:

		expansionStdAllocatorType m_allocator;
		std::vector<std::vector<elementType, expansionStdAllocatorType>> m_buffers;
		elementType* m_current_buffer;

		queue_index_t m_size;
		queue_index_t m_mask;
		const queue_index_t m_maximum_size;

		bool m_pop_possible;
		alignas(std::hardware_destructive_interference_size) std::atomic<queue_index_t> m_atomic_top;
		alignas(std::hardware_destructive_interference_size) std::atomic<queue_index_t> m_atomic_bottom;
		alignas(std::hardware_destructive_interference_size) queue_index_t m_local_bottom;

		std::vector<queue_interface*> m_steal_sources;
		typename std::vector<queue_interface*>::iterator m_next_steal_source;

	};

	template<class elementType, class expansionStdAllocatorType>
	queue_interface<elementType, expansionStdAllocatorType>::queue_interface(queue_index_t p_initial_size, queue_index_t p_maximum_size, const expansionStdAllocatorType& p_allocator)
		: m_allocator(p_allocator),
		m_buffers(),
		m_current_buffer(nullptr),
		m_size(jkutil::upto_pow2(p_initial_size)),
		m_mask(m_size - 1),
		m_maximum_size(p_maximum_size > 0 ?
			jkutil::upto_pow2(p_maximum_size) :
			jkutil::floor_pow2(std::numeric_limits<queue_index_t>::max())),
		m_pop_possible(false),
		m_atomic_top(0),
		m_atomic_bottom(0),
		m_local_bottom(0),
		m_steal_sources(),
		m_next_steal_source()
	{
		JKUTIL_ASSERT(jkutil::is_pow2(p_maximum_size) || p_maximum_size == 0);
		JKUTIL_ASSERT(jkutil::is_pow2(p_initial_size));

		m_buffers.reserve(std::numeric_limits<queue_index_t>::digits);
		expand(m_size);
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::add_stealing_source(queue_interface& p_source)
	{
		m_steal_sources.push_back(&p_source);
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::finalize_stealing_sources()
	{
		int64_t seed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		std::shuffle(m_steal_sources.begin(), m_steal_sources.end(), std::default_random_engine(static_cast<unsigned int>(seed)));
		m_next_steal_source = m_steal_sources.begin();
	}

	template<class elementType, class expansionStdAllocatorType>
	inline bool queue_interface<elementType, expansionStdAllocatorType>::try_push(sink_element_type&& p_element)
	{
		bool has_space = (remaining_space() >= 1);

		if (has_space)
		{
			m_pop_possible = true;
			push_operation(std::move(p_element));
		}

		return has_space;
	}

	template<class elementType, class expansionStdAllocatorType>
	template<class inputIteratorType>
	inline bool queue_interface<elementType, expansionStdAllocatorType>::try_push(inputIteratorType p_begin, inputIteratorType p_end)
	{
		using std::distance;

		const size_t count = distance(p_begin, p_end);

		bool has_space = true;

		if (count != 0)
		{
			has_space = (remaining_space() >= count);

			if (has_space)
			{
				m_pop_possible = true;
				push_operation(p_begin, p_end, count);
			}
		}

		return has_space;
	}

	template<class elementType, class expansionStdAllocatorType>
	template<class inputIteratorType>
	inline bool queue_interface<elementType, expansionStdAllocatorType>::try_push_move(inputIteratorType p_begin, inputIteratorType p_end)
	{
		return try_push(p_begin, p_end);
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::push(sink_element_type&& p_element)
	{
		m_pop_possible = true;

		if (remaining_space() >= 1)
		{
			push_operation(std::move(p_element));
		}
		else
		{
			if (m_size < m_maximum_size)
			{
				expand(2 * m_size, std::move(p_element));
			}
			else
			{			
				for (; remaining_space() < 1;)
				{}
				push_operation(std::move(p_element));
			}
		}
	}

	template<class elementType, class expansionStdAllocatorType>
	template<class inputIteratorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::push(inputIteratorType p_begin, inputIteratorType p_end)
	{
		using std::distance;

		const size_t count = distance(p_begin, p_end);

		if (count != 0)
		{
			m_pop_possible = true;

			if (remaining_space() >= count)
			{
				push_operation(p_begin, p_end, count);
			}
			else if (m_size < m_maximum_size)
			{
				queue_index_t new_size = jkutil::ceil_pow2(m_size + count);
				if (new_size <= m_maximum_size)
				{
					expand(jkutil::ceil_pow2(m_size + count), p_begin, p_end, count);
				}
				else
				{
					expand(m_maximum_size);
					for (; remaining_space() < count;)
					{
					}
					push_operation(p_begin, p_end, count);
				}
			}
			else
			{
				for (; remaining_space() < count;)
				{
				}
				push_operation(p_begin, p_end, count);
			}
		}
	}

	template<class elementType, class expansionStdAllocatorType>
	template<class inputIteratorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::push_move(inputIteratorType p_begin, inputIteratorType p_end)
	{
		push(p_begin, p_end);
	}

	template<class elementType, class expansionStdAllocatorType>
	inline auto queue_interface<elementType, expansionStdAllocatorType>::try_pop() -> std::optional<source_element_type>
	{
		return m_pop_possible ? pop_operation() : try_steal();
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::flush()
	{
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::push_operation(sink_element_type&& p_element)
	{
		m_current_buffer[m_local_bottom & m_mask] = std::move(p_element);
		m_local_bottom = m_local_bottom + 1;
		m_atomic_bottom.store(m_local_bottom, std::memory_order_release);
	}

	template<class elementType, class expansionStdAllocatorType>
	template<class inputIteratorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::push_operation(inputIteratorType p_begin, inputIteratorType p_end, size_t p_distance)
	{
		queue_index_t buffer_position = m_local_bottom;
		for (inputIteratorType i = p_begin; i != p_end; ++i, ++buffer_position)
		{
			m_current_buffer[buffer_position & m_mask] = *i;
		}
		m_local_bottom += p_distance;
		m_atomic_bottom.store(m_local_bottom, std::memory_order_release);
	}

	template<class elementType, class expansionStdAllocatorType>
	inline auto queue_interface<elementType, expansionStdAllocatorType>::try_steal() -> std::optional<source_element_type>
	{
		if (!m_steal_sources.empty())
		{
			auto source = *next_steal_source();

			return source->steal();
		}
		else
		{
			return std::optional<elementType>();
		}
	}

	template<class elementType, class expansionStdAllocatorType>
	inline auto queue_interface<elementType, expansionStdAllocatorType>::steal() -> std::optional<source_element_type>
	{
		queue_index_t temp_top = m_atomic_top.load(std::memory_order_acquire);
		queue_index_t temp_bottom = m_atomic_bottom.load(std::memory_order_acquire);

		std::optional<elementType> result;

		if (temp_top < temp_bottom)
		{
			result.emplace(m_current_buffer[temp_top & m_mask]);

			if (!m_atomic_top.compare_exchange_strong(temp_top, temp_top + 1, std::memory_order_relaxed))
			{
				result.reset();
			}
		}

		return result;
	}

	template<class elementType, class expansionStdAllocatorType>
	inline auto queue_interface<elementType, expansionStdAllocatorType>::pop_operation() -> std::optional<source_element_type>
	{
		std::optional<elementType> result;

		if (m_local_bottom != 0)
		{
			m_local_bottom = m_local_bottom - 1;
			m_atomic_bottom.store(m_local_bottom, std::memory_order_relaxed);

			std::atomic_thread_fence(std::memory_order_seq_cst);

			queue_index_t temp_top = m_atomic_top.load(std::memory_order_relaxed);

			if (temp_top < m_local_bottom)
			{
				result.emplace(m_current_buffer[m_local_bottom & m_mask]);
			}
			else if (temp_top == m_local_bottom)
			{
				result.emplace(m_current_buffer[m_local_bottom & m_mask]);

				queue_index_t temp = temp_top;
				if (!m_atomic_top.compare_exchange_strong(temp, temp + 1, std::memory_order_relaxed))
				{
					result.reset();
				}

				m_local_bottom = temp_top + 1;
				m_atomic_bottom.store(m_local_bottom, std::memory_order_relaxed);
			}
			else
			{
				m_pop_possible = false;
				m_local_bottom = temp_top;
				m_atomic_bottom.store(m_local_bottom, std::memory_order_relaxed);
			}
		}

		return result;
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::expand(queue_index_t p_size)
	{
		expand_operation(p_size);
		m_atomic_bottom.store(m_local_bottom, std::memory_order_release);
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::expand(queue_index_t p_size, elementType&& p_pushed_element)
	{
		expand_operation(p_size);
		push_operation(std::move(p_pushed_element));
	}

	template<class elementType, class expansionStdAllocatorType>
	template<class inputIteratorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::expand(queue_index_t p_size, inputIteratorType p_begin, inputIteratorType p_end, size_t p_distance)
	{
		expand_operation(p_size);
		push_operation(p_begin, p_end, p_count);
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::expand_operation(queue_index_t p_size)
	{
		JKUTIL_ASSERT(jkutil::is_pow2(p_size));
		JKUTIL_ASSERT(p_size > m_size);
		JKUTIL_ASSERT(m_buffers.capacity() > m_buffers.size());

		std::vector<elementType, expansionStdAllocatorType>& buffer = m_buffers.emplace_back(static_cast<const expansionStdAllocatorType&>(m_allocator));
		buffer.resize(static_cast<size_t>(p_size));

		//make queue appear empty
		queue_index_t temp_bottom = m_local_bottom;
		m_atomic_bottom.store(0, std::memory_order_relaxed);
		queue_index_t temp_top = m_atomic_top.exchange(std::numeric_limits<queue_index_t>::max(), std::memory_order_release);

		copy_buffer(buffer.data(), temp_top, temp_bottom);

		m_current_buffer = buffer.data();
		m_size = p_size;
		m_mask = p_size - 1;

		queue_index_t new_top = p_size * ((temp_top / p_size) + 1);
		m_local_bottom = new_top + temp_bottom - temp_top;
		m_atomic_bottom.store(m_local_bottom, std::memory_order_release);
		m_atomic_top.store(new_top, std::memory_order_release);
	}

	template<class elementType, class expansionStdAllocatorType>
	inline void queue_interface<elementType, expansionStdAllocatorType>::copy_buffer(elementType* p_destination, queue_index_t p_src_begin, queue_index_t p_src_end)
	{
		JKUTIL_ASSERT(p_src_begin <= p_src_end);

		//test if src range is empty		
		if (p_src_begin != p_src_end)
		{
			queue_index_t masked_begin = p_src_begin & m_mask;
			queue_index_t masked_last = (p_src_end - 1) & m_mask;
			queue_index_t masked_end = p_src_end & m_mask;

			if (masked_begin <= masked_last)
			{
				//single continuous copy (begin -> end)
				std::copy(&m_current_buffer[masked_begin], &m_current_buffer[masked_last + 1], p_destination);
			}
			else
			{
				//two seperate continuous copy (begin -> buffer_end, and buffer_start -> end)
				std::copy(&m_current_buffer[masked_begin], &m_current_buffer[m_size], p_destination);
				std::copy(&m_current_buffer[0], &m_current_buffer[masked_end], p_destination + m_size - masked_begin);
			}
		}
	}

	template<class elementType, class expansionStdAllocatorType>
	inline auto queue_interface<elementType, expansionStdAllocatorType>::remaining_space() -> queue_index_t
	{
		return m_size - (m_local_bottom - m_atomic_top.load(std::memory_order_relaxed));
	}

	template<class elementType, class expansionStdAllocatorType>
	inline auto queue_interface<elementType, expansionStdAllocatorType>::next_steal_source() -> typename std::vector<queue_interface*>::iterator
	{
		JKUTIL_ASSERT(!m_steal_sources.empty());

		auto current = m_next_steal_source;

		++m_next_steal_source;
		if (m_next_steal_source == m_steal_sources.end())
		{
			m_next_steal_source = m_steal_sources.begin();
		}

		return current;
	}

	template <class elementType, class expansionStdAllocatorType = std::allocator<elementType>>
	using queue_interface_reference = queue_reference<queue_interface<elementType, expansionStdAllocatorType>>;

	template <
		class elementType,
		class expansionStdAllocatorType = std::allocator<elementType>>
		class queue
	{
	public:

		using size_type = typename queue_interface<elementType, expansionStdAllocatorType>::queue_index_t;

		queue(size_type p_initial_size = 1, size_type p_maximum_size = 0)
			: m_initial_size(p_initial_size), m_maximum_size(p_maximum_size)
		{}

		queue(const queue&) = delete;
		queue(queue&&) = delete;
		queue& operator=(const queue&) = delete;
		queue& operator=(queue&&) = delete;

		queue_interface_reference<elementType, expansionStdAllocatorType> create_interface(const expansionStdAllocatorType& p_allocator = expansionStdAllocatorType())
		{
			return queue_interface_reference<elementType, expansionStdAllocatorType>(m_interfaces.emplace_back(m_initial_size, m_maximum_size, p_allocator));
		}

		void finalize_interfaces()
		{
			for (queue_interface<elementType, expansionStdAllocatorType>& i : m_interfaces)
			{
				for (queue_interface<elementType, expansionStdAllocatorType>& j : m_interfaces)
				{
					if (&i != &j)
					{
						i.add_stealing_source(j);
					}
				}

				i.finalize_stealing_sources();
			}
		}

	private:

		const size_type m_initial_size;
		const size_type m_maximum_size;
		std::list<queue_interface<elementType, expansionStdAllocatorType>> m_interfaces;

	};

}

#endif