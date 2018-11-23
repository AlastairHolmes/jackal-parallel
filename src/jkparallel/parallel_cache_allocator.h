#ifndef JKPARALLEL_PARALLEL_CACHE_ALLOCATOR_H
#define JKPARALLEL_PARALLEL_CACHE_ALLOCATOR_H

#include <jkparallel\queue.h>
#include <jkutil\allocator.h>
#include <jkutil\memory.h>

namespace jkparallel
{

	template <class allocatorType = jkutil::allocator>
	class parallel_cache_allocator_interface_reference
	{
	public:

		using propagate_on_container_copy_assignment = std::false_type;
		using propagate_on_container_move_assignment = std::false_type;
		using propagate_on_container_swap = std::false_type;
		using is_always_equal = std::false_type;

		parallel_cache_allocator_interface_reference(std::size_t p_cacheline_size, std::size_t p_cacheline_alignment, queue_interface_reference<void*> p_cache_interface, const allocatorType& p_allocator = allocatorType())
			: m_cacheline_size(p_cacheline_size),
			m_cacheline_alignment(p_cacheline_alignment),
			m_allocator(p_allocator),
			m_cache_interface(p_cache_interface)
		{}

		parallel_cache_allocator_interface_reference(const parallel_cache_allocator_interface_reference&) = default;
		parallel_cache_allocator_interface_reference(parallel_cache_allocator_interface_reference&&) = default;

	public:

		parallel_cache_allocator_interface_reference& operator=(const parallel_cache_allocator_interface_reference&) = delete;
		parallel_cache_allocator_interface_reference& operator=(parallel_cache_allocator_interface_reference&&) = delete;

		bool operator==(const parallel_cache_allocator_interface_reference& p_rhs) const
		{
			return m_allocator == p_rhs.m_allocator &&
				m_cache_interface == p_rhs.m_cache_interface;
		}

		bool operator!=(const parallel_cache_allocator_interface_reference& p_rhs) const
		{
			return m_allocator != p_rhs.m_allocator ||
				m_cache_interface != p_rhs.m_cache_interface;
		}

		void* allocate(std::size_t p_size, std::size_t p_alignment)
		{
			JKUTIL_ASSERT(p_size <= m_cacheline_size && p_alignment <= m_cacheline_alignment);

			std::optional<void*> memory = m_cache_interface.try_pop();

			if (memory.has_value())
			{
				return memory.value();
			}
			else
			{
				return jkutil::memory_allocate(m_allocator, m_cacheline_size, m_cacheline_alignment);
			}
		}

		void deallocate(void* p_ptr, std::size_t p_size)
		{
			JKUTIL_ASSERT(p_size <= m_cacheline_size);

			void* temp = p_ptr;

			if (!m_cache_interface.try_push(std::move(temp)))
			{
				jkutil::memory_deallocate(m_allocator, p_ptr, m_cacheline_size);
			}

		}

	private:

		const std::size_t m_cacheline_size;
		const std::size_t m_cacheline_alignment;
		allocatorType m_allocator;
		queue_interface_reference<void*> m_cache_interface;

	};

	class parallel_cache_allocator
	{
	public:

		parallel_cache_allocator(std::size_t p_cacheline_size, std::size_t p_cacheline_alignment, std::size_t p_maximum_cache_size);

		parallel_cache_allocator(const parallel_cache_allocator&) = delete;
		parallel_cache_allocator(parallel_cache_allocator&&) = delete;
		parallel_cache_allocator& operator=(const parallel_cache_allocator&) = delete;
		parallel_cache_allocator& operator=(parallel_cache_allocator&&) = delete;

		template <class allocatorType>
		parallel_cache_allocator_interface_reference<allocatorType> create_interface(const allocatorType& p_allocator = allocatorType())
		{
			return parallel_cache_allocator_interface_reference<allocatorType>(m_cacheline_size, m_cacheline_alignment, m_cache.create_interface(), p_allocator);
		}

		void finalize_interfaces();

	private:

		const std::size_t m_cacheline_size;
		const std::size_t m_cacheline_alignment;
		queue<void*> m_cache;

	};

}

#endif