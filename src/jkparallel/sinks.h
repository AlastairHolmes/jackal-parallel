#ifndef JKPARALLEL_SINKS_H
#define JKPARALLEL_SINKS_H

#include <vector>
#include <type_traits>

namespace jkparallel
{

	template <class referencedSinkType>
	class reference_sink
	{
	public:

		using sink_element_type = typename referencedSinkType::sink_element_type;
		using is_staged_sink = std::false_type;

		reference_sink(referencedSinkType& p_sink)
			: m_sink(p_sink)
		{}

		bool try_push(sink_element_type&& p_element)
		{
			return m_sink.try_push(std::move(p_element));
		}

		template <class inputIteratorType>
		bool try_push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			return m_sink.try_push(p_begin, p_end);
		}

		template <class inputIteratorType>
		bool try_push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			return m_sink.try_push_move(p_begin, p_end);
		}

		void push(sink_element_type&& p_element)
		{
			m_sink.push(std::move(p_element));
		}

		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_sink.push(p_begin, p_end);
		}

		template <class inputIteratorType>
		void push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_sink.push_move(p_begin, p_end);
		}

	protected:

		referencedSinkType& m_sink;

	};

	template <class referencedStagedSinkType>
	class reference_staged_sink : public reference_sink<referencedStagedSinkType>
	{
	public:

		static_assert(referencedStagedSinkType::is_staged_sink::value, "'referencedStagedSinkType' must be a 'staged sink'.");

		using sink_element_type = typename referencedStagedSinkType::sink_element_type;
		using is_staged_sink = std::true_type;

		reference_staged_sink(referencedStagedSinkType& p_sink)
			: reference_sink<referencedStagedSinkType>(p_sink)
		{}

		void stage_push(sink_element_type&& p_element)
		{
			m_sink.stage_push(std::move(p_element));
		}

		template <class inputIteratorType>
		void stage_push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_sink.stage_push(p_begin, p_end);
		}

		template <class inputIteratorType>
		void stage_push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_sink.stage_push_move(p_begin, p_end);
		}

		void finalize_push()
		{
			m_sink.finalize_push();
		}

		bool try_finalize_push()
		{
			return m_sink.try_finalize_push();
		}

	};

	template <class sinkType, class stdAllocatorType = std::allocator<typename sinkType::sink_element_type>>
	class simple_sink_stager
	{
	public:

		using sink_element_type = typename sinkType::sink_element_type;
		using is_staged_sink = std::false_type;

		simple_sink_stager(sinkType&& p_sink, const stdAllocatorType& p_allocator = stdAllocatorType())
			: m_sink(std::move(p_sink)), m_staging_buffer(p_allocator)
		{}

		simple_sink_stager(const simple_sink_stager&) = default;
		simple_sink_stager(simple_sink_stager&&) = default;

		simple_sink_stager& operator=(const simple_sink_stager&) = default;
		simple_sink_stager& operator=(simple_sink_stager&&) = default;

		bool try_push(sink_element_type&& p_element)
		{
			return m_sink.try_push(std::move(p_element));
		}

		template <class inputIteratorType>
		bool try_push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			return m_sink.try_push(p_begin, p_end);
		}

		template <class inputIteratorType>
		bool try_push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			return m_sink.try_push_move(p_begin, p_end);
		}

		void push(sink_element_type&& p_element)
		{
			m_sink.push(std::move(p_element));
		}

		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_sink.push(p_begin, p_end);
		}

		template <class inputIteratorType>
		void push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_sink.push_move(p_begin, p_end);
		}

		void stage_push(sink_element_type&& p_element)
		{
			m_staging_buffer.push_back(std::move(p_element));
		}

		template <class inputIterator>
		void stage_push(inputIterator p_begin, inputIterator p_end)
		{
			using std::distance;

			auto difference = distance(p_begin, p_end);
			m_staging_buffer.reserve(difference);

			for (auto i = p_begin; i != p_end; ++i)
			{
				m_staging_buffer.emplace_back(*i);
			}
		}

		template <class inputIteratorType>
		void stage_push_move(inputIteratorType p_begin, inputIteratorType p_end)
		{
			using std::distance;

			auto difference = distance(p_begin, p_end);
			m_staging_buffer.reserve(difference);

			for (auto i = p_begin; i != p_end; ++i)
			{
				m_staging_buffer.emplace_back(std::move(*i));
			}
		}

		void finalize_push()
		{
			m_sink.push_move(m_staging_buffer.begin(), m_staging_buffer.end());
			m_staging_buffer.clear();
		}

		bool try_finalize_push()
		{
			bool complete = m_sink.try_push_move(m_staging_buffer.begin(), m_staging_buffer.end());
			if (complete)
			{
				m_staging_buffer.clear();
			}
			return complete;
		}

	protected:

		sinkType& get_internal_sink()
		{
			return m_sink;
		}

		const sinkType& get_internal_sink() const
		{
			return m_sink;
		}

	private:

		sinkType m_sink;
		std::vector<sink_element_type, stdAllocatorType> m_staging_buffer;

	};

}

#endif