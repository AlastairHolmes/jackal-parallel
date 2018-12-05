#ifndef JKPARALLEL_PARALLEL_ALLOCATOR_H
#define JKPARALLEL_PARALLEL_ALLOCATOR_H

#include <jkparallel\parallel_cache_allocator.h>
#include <jkutil\bucket_allocator.h>
#include <jkutil\allocator_reference.h>
#include <jkutil\cache_allocator.h>
#include <functional>
#include <list>

namespace jkparallel
{

	//class parallel_allocator_interface
	//{
	//public:

	//	using propagate_on_container_copy_assignment = std::false_type;
	//	using propagate_on_container_move_assignment = std::false_type;
	//	using propagate_on_container_swap = std::false_type;
	//	using is_always_equal = std::false_type;

	//	template <class bucketAllocatorMapConstructorCallable>
	//	parallel_allocator_interface(std::size_t p_buckets, std::size_t p_smallest_bucket, std::size_t p_bucket_power_step, bucketAllocatorMapConstructorCallable&& p_allocator_constructor)
	//		: m_allocator(p_buckets, p_smallest_bucket, p_bucket_power_step, std::forward<bucketAllocatorMapConstructorCallable>(p_allocator_constructor))
	//	{}

	//	parallel_allocator_interface(const parallel_allocator_interface&) = delete;
	//	parallel_allocator_interface(parallel_allocator_interface&&) = delete;

	//	parallel_allocator_interface& operator=(const parallel_allocator_interface&) = delete;
	//	parallel_allocator_interface& operator=(parallel_allocator_interface&&) = delete;

	//public:

	//	bool operator==(const parallel_allocator_interface& p_rhs) const
	//	{
	//		return this == &p_rhs;
	//	}

	//	bool operator!=(const parallel_allocator_interface& p_rhs) const
	//	{
	//		return this != &p_rhs;
	//	}

	//	void* allocate(std::size_t p_size, std::size_t p_alignment)
	//	{
	//		m_allocator.a
	//	}

	//	void deallocate(void* p_ptr, std::size_t p_size)
	//	{

	//	}

	//private:

	//	jkutil::bucket_allocator<parallel_cache_allocator_interface_reference<>> m_allocator;

	//};

	using parallel_allocator_interface = jkutil::bucket_allocator<jkutil::cache_allocator<parallel_cache_allocator_interface_reference<>>>;

	using parallel_allocator_interface_reference = jkutil::opaque_allocator_immutable_reference<parallel_allocator_interface>;

	class parallel_allocator
	{

	public:

		parallel_allocator(
			std::function<std::size_t(std::size_t, std::size_t)> p_shared_cache_size_callable,
			std::function<std::size_t(std::size_t, std::size_t)> p_private_cache_size_callable,
			std::size_t p_buckets,
			std::size_t p_smallest_bucket,
			std::size_t p_bucket_power_step = 1);

		parallel_allocator(const parallel_allocator&) = delete;
		parallel_allocator(parallel_allocator&&) = delete;

		parallel_allocator& operator=(const parallel_allocator&) = delete;
		parallel_allocator& operator=(parallel_allocator&&) = delete;

		parallel_allocator_interface_reference create_interface();

		void finalize_interfaces();

	private:

		std::function<std::size_t(std::size_t, std::size_t)> m_shared_cache_size_callable;
		std::function<std::size_t(std::size_t, std::size_t)> m_private_cache_size_callable;
		const std::size_t m_buckets;
		const std::size_t m_minimum_bucket_size;
		const std::size_t m_bucket_power_step;
		std::list<parallel_allocator_interface> m_interfaces;
		std::list<parallel_cache_allocator> m_allocators;

	};

}

#endif