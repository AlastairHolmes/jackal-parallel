#ifndef JKPARALLEL_SINKS_H
#define JKPARALLEL_SINKS_H


#include <type_traits>

namespace jkparallel
{

	template <class referencedSinkType>
	class reference_sink
	{
	public:

		using element_type = typename referencedSinkType::element_type;
		using is_staged_sink = std::false_type;

		reference_sink(referencedSinkType& p_sink)
			: m_sink(&p_sink)
		{}

		void push(element_type&& p_element)
		{
			m_sink.push(std::move(p_element));
		}

		template <class inputIteratorType>
		void push(inputIteratorType p_begin, inputIteratorType p_end)
		{
			m_sink.push(p_begin, p_end);
		}

	protected:

		referencedSinkType* m_sink;

	};

	template <class referencedStagedSinkType>
	class reference_staged_sink : public reference_sink<referencedStagedSinkType>
	{
	public:

		static_assert(referencedStagedSinkType::is_staged_sink::value, "'referencedStagedSinkType' must be a 'staged sink'.");

		using element_type = typename referencedStagedSinkType::element_type;
		using is_staged_sink = std::true_type;

		reference_staged_sink(referencedStagedSinkType& p_sink)
			: reference_sink<referencedStagedSinkType>(p_sink)
		{}

		void stage_push(element_type&& p_element)
		{
			m_sink.stage_push(std::move(p_element));
		}

		template <class inputIterator>
		void stage_push(inputIterator p_begin, inputIterator p_end)
		{
			m_sink.stage_push(p_begin, p_end);
		}

		void finalize_push()
		{
			m_sink.finalize_push();
		}

	};

	template <class sinkType, class allocatorType = jkutil::allocator>
	class simple_sink_stager
	{
	public:



	private:

		sinkType m_sink;

	};

}

#endif