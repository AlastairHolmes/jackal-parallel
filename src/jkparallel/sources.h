#ifndef JKPARALLEL_SOURCES_H
#define JKPARALLEL_SOURCES_H

#include <optional>

namespace jkparallel
{

	template <class referencedSourceType>
	class reference_source
	{
	public:

		using source_element_type = typename referencedSourceType::source_element_type;

		reference_source(referencedSourceType& p_source)
			: m_source(p_source)
		{

		}

		std::optional<source_element_type> try_pop()
		{
			return m_source.try_pop();
		}

		void flush()
		{
			m_source.flush();
		}

	private:

		referencedSourceType& m_source;
	};

}

#endif