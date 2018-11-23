#include <jkparallel\parallel_allocator.h>

namespace jkparallel
{

	parallel_allocator::parallel_allocator(std::size_t p_buckets, std::size_t p_smallest_bucket, std::size_t p_bucket_power_step)
		: m_buckets(p_buckets),
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
				return m_allocators.emplace_back(p_bucket_size, p_bucket_alignment, 64).create_interface(jkutil::allocator());
			});
		}
		else
		{
			return m_interfaces.emplace_back(m_buckets, m_minimum_bucket_size, m_bucket_power_step,
			[iterator = m_allocators.begin(), end_iterator = m_allocators.end()](std::size_t p_bucket_index, std::size_t p_bucket_size, std::size_t p_bucket_alignment) mutable
			{
				JKUTIL_ASSERT(iterator != end_iterator);
				return (iterator++)->create_interface(jkutil::allocator());
			});
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