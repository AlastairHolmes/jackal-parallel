#include <jkparallel\parallel_cache_allocator.h>

namespace jkparallel
{

	parallel_cache_allocator::parallel_cache_allocator(std::size_t p_cacheline_size, std::size_t p_cacheline_alignment, std::size_t p_maximum_cache_size)
		: m_cacheline_size(p_cacheline_size),
		m_cacheline_alignment(p_cacheline_alignment),
		m_cache(p_maximum_cache_size, p_maximum_cache_size)
	{}

	void parallel_cache_allocator::finalize_interfaces()
	{
		m_cache.finalize_interfaces();
	}

}