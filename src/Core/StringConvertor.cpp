module;

#include <cuchar>
#include <locale>

export module Nyaanwork.Core.StringConvertor;

import Nyaanwork.Core.Types;

#ifdef _WIN64
	#define LOCALE // FIXME: In MinGW not work std::locale for en_US.utf8
#else
	#define LOCALE "en_US.UTF-8"
#endif

export namespace Nyaanwork
{
	template<typename BigChar>
	class StringConvertor
	{
	private:
		using CodeCvt = std::codecvt<BigChar, char, std::mbstate_t>;

	public:
		StringConvertor(std::locale locale = std::locale(LOCALE))
			: m_locale(std::move(locale)),
			  m_facet(std::use_facet<CodeCvt>(m_locale)) {}

		Str convert(std::basic_string_view<BigChar> wstr)
			{ return convert<BigChar, char, &CodeCvt::out>(wstr); }

		std::basic_string<BigChar> convert(StrV str)
			{ return convert<char, BigChar, &CodeCvt::in>(str); }

	private:
		template<typename I, typename O, auto method>
		std::basic_string<O> convert(std::basic_string_view<I> in)
		{
			static constexpr usize factor = sizeof(I);

			std::basic_string<O> out(in.size()*factor+1, '\0');

			std::mbstate_t state = {};
			const I* from_next;
			O* to_next;
			(m_facet.*method)(state, in.begin(), in.end(), from_next,
							  out.begin().base(), out.end().base(), to_next);
			out.resize(to_next - out.begin().base());

			return out;
		}

		std::locale m_locale;
		decltype(std::use_facet<CodeCvt>({})) m_facet;
	};

	using StringConvertor_WChar = StringConvertor<wchar_t>;
	using StringConvertor_UTF16 = StringConvertor<char16_t>;
	using StringConvertor_UTF32 = StringConvertor<char32_t>;
}
