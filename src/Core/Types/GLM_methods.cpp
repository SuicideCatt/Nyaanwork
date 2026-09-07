module;

#include <Core/GLM.hpp>

#include <format>
#include <ostream>

export module Nyaanwork.Core.Types:GLM_methods;
import :Numbers;

using namespace Nyaanwork::Types;

template<typename Char>
struct Symbols
{
	static constexpr Char open = static_cast<Char>('{');
	static constexpr Char close = static_cast<Char>('}');

	static constexpr std::array split_data = {
		static_cast<Char>(','),
		static_cast<Char>(' '),
		static_cast<Char>('\0'),
	};

	static constexpr std::basic_string_view<Char> split = {split_data.data(), 2};
};

export namespace glm
{
	using glm::operator+;
	using glm::operator-;
	using glm::operator*;
	using glm::operator/;

	using glm::operator%;
	using glm::operator&;
	using glm::operator|;
	using glm::operator^;
	using glm::operator<<;
	using glm::operator>>;
	using glm::operator~;

	using glm::operator&&;
	using glm::operator||;
	using glm::operator==;
	using glm::operator!=;

	template<typename Char, is_arithmetic T, qualifier q>
	auto operator<<(std::basic_ostream<Char>& out, const vec<1, T, q>& vec)
		-> std::basic_ostream<Char>&
	{
		using S = Symbols<Char>;
		return out << S::open << vec.x << S::close;
	}

	template<typename Char, is_arithmetic T, qualifier q>
	auto operator<<(std::basic_ostream<Char>& out, const vec<2, T, q>& vec)
		-> std::basic_ostream<Char>&
	{
		using S = Symbols<Char>;
		return out << S::open << vec.x << S::split << vec.y << S::close;
	}

	template<typename Char, is_arithmetic T, qualifier q>
	auto operator<<(std::basic_ostream<Char>& out, const vec<3, T, q>& vec)
		-> std::basic_ostream<Char>&
	{
		using S = Symbols<Char>;
		return out << S::open << vec.x << S::split << vec.y << S::split
					<< vec.z << S::close;
	}

	template<typename Char, is_arithmetic T, qualifier q>
	auto operator<<(std::basic_ostream<Char>& out, const vec<4, T, q>& vec)
		-> std::basic_ostream<Char>&
	{
		using S = Symbols<Char>;
		return out << S::open << vec.x << S::split << vec.y << S::split
					<< vec.z << S::split << vec.w << S::close;
	}

	template<size_t i, length_t l, is_arithmetic T, qualifier q>
	requires (i < l)
	constexpr T& get(vec<l, T, q>& vec) noexcept
		{ return vec[i]; }

	template<size_t i, length_t l, is_arithmetic T, qualifier q>
	requires (i < l)
	constexpr T get(const vec<l, T, q>& vec) noexcept
		{ return vec[i]; }
}

export namespace std
{
	using std::tuple_size;
	using std::tuple_element;

	template<glm::length_t l, Nyaanwork::Types::is_arithmetic Type, glm::qualifier q>
	struct tuple_size<glm::vec<l, Type, q>> : std::integral_constant<size_t, l> {};

	template<size_t i, glm::length_t l, Nyaanwork::Types::is_arithmetic Type,
			 glm::qualifier q>
	struct tuple_element<i, glm::vec<l, Type, q>>
	{
		using type = Type;
	};

	template<glm::length_t l, Nyaanwork::Types::is_arithmetic T,
			 glm::qualifier q, typename Char>
	struct formatter<glm::vec<l, T, q>, Char>
	{
		constexpr formatter()
		{
			for (glm::length_t i = 0; i < l; ++i)
				id[i] = i;
		}

		template<typename CharP>
		using ParseCnt = std::basic_format_parse_context<CharP>;

		template<typename Out, typename CharP>
		using FormatCnt = std::basic_format_context<Out, CharP>;

		template<typename CharP>
		constexpr ParseCnt<CharP>::iterator parse(ParseCnt<CharP>& ctx)
		{
			auto it = ctx.begin();
			if (it == ctx.end())
				return it;

			auto s = [this](u8 id)
			{
				if (!(i < 4))
				{
					throw std::format_error("Invalid format arg: "
											"very big vec out.");
				}
				this->id[i++] = id;
			};

			for (; it != ctx.end(); ++it)
			{
				switch (auto c = *it)
				{
				case '!':
					quoted = !quoted;
					break;

				case ',':
				case ' ':
					spliter = c;
					break;

				case 'x':
					s(0);
					break;
				case 'y':
					if constexpr (l < 2)
					{
						throw std::format_error("Invalid format arg: "
												"vec can't contain y.");
					}
					s(1);
					break;
				case 'z':
					if constexpr (l < 3)
					{
						throw std::format_error("Invalid format arg: "
												"vec can't contain z.");
					}
					s(2);
					break;
				case 'w':
					if constexpr (l < 4)
					{
						throw std::format_error("Invalid format arg: "
												"vec can't contain w.");
					}
					s(3);
					break;

				case '}':
					return it;

				default:
					throw std::format_error("Invalid format arg for vec");
				};
			}

			return it;
		}

		template<typename Out, typename CharP>
		FormatCnt<Out, CharP>::iterator
			format(glm::vec<l, T, q> s, FormatCnt<Out, CharP>& ctx) const
		{
			std::string str;

			auto spliter = this->spliter == ','?
				std::string_view(", ") : std::string_view(" ");

			auto i = this->i;
			if (i == 0)
				i = static_cast<u8>(l);

			static constexpr std::string_view vec2 = "{1}{0}{2}";
			static constexpr std::string_view vec3 = "{1}{0}{2}{0}{3}";
			static constexpr std::string_view vec4 = "{1}{0}{2}{0}{3}{0}{4}";

			switch(i)
			{
			case 1:
				str = std::format("{}", s[id[0]]);
				break;
			case 2:
				str = std::format(vec2, spliter, s[id[0]], s[id[1]]);
				break;
			case 3:
				str = std::format(vec3, spliter, s[id[0]], s[id[1]], s[id[2]]);
				break;

			default:
				str = std::format(vec4, spliter, s[id[0]], s[id[1]],
												 s[id[2]], s[id[3]]);
				break;
			}

			if (quoted)
				str = std::format("{}{}{}", '{', std::move(str), '}');

			return std::ranges::copy(str, ctx.out()).out;
		}

		bool quoted = true;
		u8 i = 0;
		u8 id[4] = {};
		char spliter = ',';
	};
}

export namespace Nyaanwork::Types
{
	using glm::get;

	template<glm::length_t l, Nyaanwork::Types::is_arithmetic Type, glm::qualifier q>
	std::string to_string(glm::vec<l, Type, q> vec)
		{ return std::format("{}", vec); }
}
