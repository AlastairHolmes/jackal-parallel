#include <jkparallel\queue.h>
#include <jkparallel\async.h>
#include <jkparallel\blocking_queue.h>
#include <jkparallel\mpsc_queue.h>
#include <jkparallel\parallel_allocator.h>
#include <jkparallel\thread.h>
#include <jkparallel\dependent.h>
#include <jkparallel\task_graph.h>
#include <iostream>
#include <future>

int main()
{

	jkparallel::task_system ts;

	auto tsi = ts.create_interface();

	ts.finalize_interfaces();

	std::chrono::duration<double> fp_ms;


	auto t1 = std::chrono::high_resolution_clock::now();

	jkparallel::task_graph tg;

	jkparallel::node* prev = nullptr;
	for (std::size_t i = 0; i < 10000; ++i)
	{
		jkparallel::node* task = tg.create_task([](jkparallel::task_system_interface& p_inteface, jkparallel::dependent& p_end_dependent)
		{
			//std::cout << "Hello";
		});

		if (prev)
		{
			prev->prepend(*task);
		}
		prev = task;
	}

	tg.staged_submit(tsi);
	tsi.finalize_push();

	for (std::size_t i = 0; i < 10000; ++i)
	{
		tsi.try_run_task();
	}

	auto t2 = std::chrono::high_resolution_clock::now();

	fp_ms = t2 - t1;

	return fp_ms.count() * 1000000;
}

/*constexpr uint64_t sum_range(uint64_t b, uint64_t e)
{
	return((b + e)*((e - b) + 1)) / 2;
}*/

//static const uint64_t answer = sum_range(0, 980075) + sum_range(0, 800566);

/*int main()
{

	jkparallel::thread t;

	t.begin([]()
	{
		throw 8;
	});

	t.join();

	jkparallel::parallel_allocator alloc = jkparallel::parallel_allocator(4,32,1);

	auto x = alloc.create_interface();
	auto y = alloc.create_interface();

	alloc.finalize_interfaces();

	jkparallel::mpsc_queue<int> aaa(1);

	//aaa.register_producer();

	jkparallel::queue<jkparallel::base_callable*> callable_queue = jkparallel::queue<jkparallel::base_callable*>(jkutil::ceil_pow2((uint32_t)1));

	auto cq_interface = callable_queue.create_interface();
	auto cq_processor = callable_queue.create_interface();

	callable_queue.finalize_interfaces();

	auto async = jkparallel::async<jkparallel::queue_interface_reference<jkparallel::base_callable*>, jkparallel::spmc_queue<jkparallel::base_callback*>>(
		cq_interface,
		jkparallel::spmc_queue<jkparallel::base_callback*>());

	async.submit([]()
	{
		std::cout << "Hello World " << std::this_thread::get_id() << std::endl;
	},
	[]()
	{
		std::cout << "Bye World " << std::this_thread::get_id() << std::endl;
	},
	[](std::exception_ptr)
	{
		std::cout << "Exception" << std::endl;
	});

	async.submit([]()
	{
		std::cout << "Hello" << std::this_thread::get_id() << std::endl;
	},
		[]()
	{
		std::cout << "Bye" << std::this_thread::get_id() << std::endl;
	},
		[](std::exception_ptr)
	{
		std::cout << "Exception" << std::endl;
	});

	auto processor = std::async(std::launch::async, [cq_processor]() mutable
	{
		std::uint32_t found = 0;
		while (found < 2)
		{
			auto x = cq_processor.try_pop();
			if (x.has_value())
			{
				x.value()->operator()();
				++found;
			}
		}
	});

	processor.wait();

	async.process_callbacks();


	return 0;


	//auto t1 = std::chrono::high_resolution_clock::now();

	//for (uint32_t k = 0; k < 20; ++k)
	//{

	//	jkparallel::queue<int> queue = jkparallel::queue<int>(jkutil::ceil_pow2((uint32_t)1));

	//	auto a = queue.create_interface();
	//	auto b = queue.create_interface();
	//	auto c = queue.create_interface();
	//	auto d = queue.create_interface();

	//	queue.finalize_interfaces();

	//	std::atomic<uint64_t> count = 0;

	//	auto producer1_out = std::async(std::launch::async, [a, &count]() mutable
	//	{
	//		for (uint32_t i = 0; i < 980076; ++i)
	//		{
	//			a.push(std::move(i));
	//		}

	//		while (count.load(std::memory_order_relaxed) != answer)
	//		{
	//			auto result = a.try_pop();

	//			if (result.has_value())
	//			{
	//				count.fetch_add(result.value(), std::memory_order_relaxed);
	//			}
	//		}

	//	});

	//	auto producer2_out = std::async(std::launch::async, [b, &count]() mutable
	//	{
	//		for (uint32_t i = 0; i < 800567; ++i)
	//		{
	//			b.push(std::move(i));
	//		}

	//		while (count.load(std::memory_order_relaxed) != answer)
	//		{
	//			auto result = b.try_pop();

	//			if (result.has_value())
	//			{
	//				count.fetch_add(result.value(), std::memory_order_relaxed);
	//			}
	//		}

	//	});

	//	auto consumer1_out = std::async(std::launch::async, [c, &count]() mutable
	//	{
	//		while (count.load(std::memory_order_relaxed) != answer)
	//		{
	//			auto result = c.try_pop();

	//			if (result.has_value())
	//			{
	//				count.fetch_add(result.value(), std::memory_order_relaxed);
	//			}
	//		}
	//	});

	//	auto consumer2_out = std::async(std::launch::async, [d, &count]() mutable
	//	{
	//		uint64_t largest1 = 0;

	//		while (count.load(std::memory_order_relaxed) != answer)
	//		{
	//			auto result = d.try_pop();

	//			if (result.has_value())
	//			{
	//				if (result.value() > largest1)
	//				{
	//					largest1 = result.value();
	//				}
	//				count.fetch_add(result.value(), std::memory_order_relaxed);
	//			}
	//		}
	//	});

	//	consumer1_out.wait();
	//	consumer2_out.wait();
	//	producer2_out.wait();
	//	producer1_out.wait();

	//	//std::cout << count << std::endl;

	//	if (count != answer)
	//	{
	//		std::cout << "ERROR" << std::endl;
	//		return -1;
	//	}

	//}


	//auto t2 = std::chrono::high_resolution_clock::now();

	//std::chrono::duration<double> fp_ms = t2 - t1;

	//return 20 * 2 * (980076 + 800567) / fp_ms.count();
}*/