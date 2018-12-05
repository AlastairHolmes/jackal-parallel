#include <jkparallel\parallel_allocator.h>

namespace jkparallel
{

	parallel_allocator::parallel_allocator(
		std::function<std::size_t(std::size_t, std::size_t)> p_shared_cache_size_callable,
		std::function<std::size_t(std::size_t, std::size_t)> p_private_cache_size_callable,
		std::size_t p_buckets,
		std::size_t p_smallest_bucket,
		std::size_t p_bucket_power_step)
		: m_shared_cache_size_callable(p_shared_cache_size_callable),
		m_private_cache_size_callable(p_private_cache_size_callable),
		m_buckets(p_buckets),
		m_minimum_bucket_size(p_smallest_bucket),
		m_bucket_power_step(p_bucket_power_step)
	{
	}

	parallel_allocator_interface_reference parallel_allocator::create_interface()
	{
		if (m_allocators.empty())
		{
			return m_interfaces.emplace_back(m_buckets, m_minimum_bucket_size, m_bucket_power_step,
			[this](std::size_t p_bucket_index, std::size_t p_bucket_size, std::size_t p_bucket_alignment)
			{
				return jkutil::cache_allocator<parallel_cache_allocator_interface_reference<>>(p_bucket_size, p_bucket_size, m_private_cache_size_callable(p_bucket_index, p_bucket_size), m_allocators.emplace_back(p_bucket_size, p_bucket_alignment, m_shared_cache_size_callable(p_bucket_index, p_bucket_size)).create_interface(jkutil::allocator()));
			},
			jkutil::allocator());
		}
		else
		{
			return m_interfaces.emplace_back(m_buckets, m_minimum_bucket_size, m_bucket_power_step,
			[iterator = m_allocators.begin(), end_iterator = m_allocators.end()](std::size_t p_bucket_index, std::size_t p_bucket_size, std::size_t p_bucket_alignment) mutable
			{
				JKUTIL_ASSERT(iterator != end_iterator);
				return jkutil::cache_allocator<parallel_cache_allocator_interface_reference<>>(p_bucket_size, p_bucket_size, 64, (iterator++)->create_interface(jkutil::allocator()));
			},
			jkutil::allocator());
		}
	}

	void parallel_allocator::finalize_interfaces()
	{
		for (auto& allocator : m_allocators)
		{
			allocator.finalize_interfaces();
		}
	}

}