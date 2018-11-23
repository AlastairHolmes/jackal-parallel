#ifndef JKPARALLEL_QUEUES_H
#define JKPARALLEL_QUEUES_H

#include <type_traits>
#include <optional>

namespace jkparallel
{

	template <class queueType>
	class queue_reference
	{
	public:

		using source_element_type = typename queueType::source_element_type;
		using sink_element_type = typename queueType::sink_element_type;
		using is_staged_sink = std::false_type;

		queue_reference(queueType& p_queue)
			: m_queue(p_queue)
		{

		}

		queue_reference(const queue_reference&) = default;
		queue_reference(queue_reference&&) = default;

		queue_reference& operator=(const queue_reference&) = delete;
		queue_reference& operator=(queue_reference&&) = delete;

		bool operator==(const queue_reference& p_rhs) const
		{
			return m_queue == p_rhs.m_queue;
		}

		bool operator!=(const queue_reference& p_rhs) const
		{
			return m_queue != p_rhs.m_queue;
		}

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
			return m_queue.try_pop();
		}

		void flush()
		{
			m_queue.flush();
		}

	private:

		queueType& m_queue;

	};

	template <class queueType>
	class staged_queue_reference
	{
	public:

		static_assert(queueType::is_staged_sink::value);

		using source_element_type = typename queueType::source_element_type;
		using sink_element_type = typename queueType::sink_element_type;
		using is_staged_sink = std::true_type;

		staged_queue_reference(queueType& p_queue)
			: m_queue(p_queue)
		{

		}

		staged_queue_reference(const staged_queue_reference&) = default;
		staged_queue_reference(staged_queue_reference&&) = default;

		staged_queue_reference& operator=(const staged_queue_reference&) = delete;
		staged_queue_reference& operator=(staged_queue_reference&&) = delete;

		bool operator==(const staged_queue_reference& p_rhs) const
		{
			return m_queue == p_rhs.m_queue;
		}

		bool operator!=(const staged_queue_reference& p_rhs) const
		{
			return m_queue != p_rhs.m_queue;
		}

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
			return m_queue.try_push_move(p_begin, p_end);;
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

		void stage_push(sink_element_type&& p_element)
		{
			m_queue.stage_push(std::move(p_element));
		}

		template <class inputIteratorType>
		void stage_push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_queue.stage_push(p_begin, p_end);
		}

		template <class inputIteratorType>
		void stage_push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_queue.stage_push_move(p_begin, p_end);
		}

		void finalize_push()
		{
			m_queue.finalize_push();
		}

		bool try_finalize_push()
		{
			return m_queue.try_finalize_push();
		}

		std::optional<source_element_type> try_pop()
		{
			return m_queue.try_pop();
		}

		void flush()
		{
			m_queue.flush();
		}

	private:

		queueType& m_queue;

	};

	template <class queueType>
	class caching_queue
	{
	public:

		static_assert(std::is_same_v<typename queueType::sink_element_type, typename queueType::source_element_type>);

		using sink_element_type = typename queueType::sink_element_type;
		using source_element_type = typename queueType::source_element_type;
		using is_staged_sink = std::false_type;

		caching_queue(queueType&& p_queue)
			: m_queue(std::move(p_queue))
		{}

		bool try_push(sink_element_type&& p_element)
		{
			if (!m_cache.has_value())
			{
				m_cache.emplace(std::move(p_element));
				return true;
			}
			else
			{
				return m_queue.try_push(std::move(p_element));
			}
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
			if (!m_cache.has_value())
			{
				m_cache.emplace(std::move(p_element));
			}
			else
			{
				m_queue.push(std::move(p_element));
			}
		}

		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			if (p_begin != p_end)
			{
				if (!m_cache.has_value())
				{
					m_cache.emplace(*p_begin);
					++p_begin;
				}
				m_queue.push(p_begin, p_end);
			}
		}

		template <class inputIteratorType>
		void push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			if (p_begin != p_end)
			{
				if (!m_cache.has_value())
				{
					m_cache.emplace(*p_begin);
					++p_begin;
				}
				m_queue.push_move(p_begin, p_end);
			}
		}

		std::optional<source_element_type> try_pop()
		{
			std::optional<source_element_type> result;

			if(m_cache.has_value())
			{
				result = std::move(m_cache);
				m_cache.reset();
			}
			else
			{
				result = std::move(m_queue.try_pop());
			}

			return result;
		}

		void flush()
		{
			if (m_cache.has_value())
			{
				m_queue.push(std::move(m_cache.value()));
				m_cache.reset();
			}
			m_queue.flush();
		}

	private:

		std::optional<sink_element_type> m_cache;
		queueType m_queue;

	};

}

#endif