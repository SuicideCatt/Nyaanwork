module;

#include <algorithm>
#include <format>

export module Nyaanwork.Asset.Image:Pixel.std_integration;
import :Pixel;

namespace Nyaanwork::Asset::Pixels
{
	template<typename T>
	Str get_pixel_fd_format(auto&& ch, u8 i, char spliter, auto& id)
	{
		auto fmt = [spliter](auto vec)
		{
			if (spliter == ',')
				return std::format("{:!,}", vec);
			else
				return std::format("{:! }", vec);
		};

		switch(i)
		{
		case 1:
			{
				auto vec = static_cast<T>(R(ch(id[0])));
				return std::format("{}", vec);
			}
		case 2:
			{
				auto vec = static_cast<vec2<T>>(RA(ch(id[0]), ch(id[1])));
				return fmt(vec);
			}
		case 3:
			{
				auto vec = static_cast<vec3<T>>(RGB(ch(id[0]), ch(id[1]),
													ch(id[2])));
				return fmt(vec);
			}
		case 4:
			{
				auto vec = static_cast<vec4<T>>(RGBA(ch(id[0]), ch(id[1]),
													 ch(id[2]), ch(id[3])));
				return fmt(vec);
			}

		default:
			return "";
		}
	}

	template<typename Char>
	struct Symbols
	{
		static constexpr Char open = static_cast<Char>('#');
	};
}

export template<Nyaanwork::Asset::Pixels::is_pixel Pixel, typename Char>
struct std::formatter<Pixel, Char>
{
	constexpr formatter()
	{
		using namespace Nyaanwork;

		for (u8 i = 0; i < static_cast<u8>(Pixel::channels)+1; ++i)
			id[i] = i;
	}

	template<typename CharP>
	using ParseCnt = std::basic_format_parse_context<CharP>;

	template<typename Out, typename CharP>
	using FormatCnt = std::basic_format_context<Out, CharP>;

	template<typename CharP>
	constexpr ParseCnt<CharP>::iterator parse(ParseCnt<CharP>& ctx)
	{
		using namespace Nyaanwork;
		using namespace Nyaanwork::Asset::Pixels;

		auto it = ctx.begin();
		if (it == ctx.end())
			return it;

		auto s = [this](u8 id)
		{
			if (!(i < 4))
				throw std::format_error("Invalid format arg: very pixel out.");
			this->id[i++] = id;
		};

		for (; it != ctx.end(); ++it)
		{
			switch (auto c = *it)
			{
			case '!':
				quoted = !quoted;
				break;

			case 'h':
			case 'f':
			case 'd':
				type = c;
				break;

			case 'L':
			case 'l':
				capitalize = c == 'L';
				break;

			case ',':
			case ' ':
				spliter = c;
				break;

			case 'r':
				s(0);
				break;
			case 'g':
				if constexpr (is_single_color<Pixel>)
				{
					throw std::format_error("Invalid format arg: "
											"pixel can't contain g channel.");
				}
				s(1);
				break;
			case 'b':
				if constexpr (is_single_color<Pixel>)
				{
					throw std::format_error("Invalid format arg: "
											"pixel can't contain b channel.");
				}
				s(2);
				break;
			case 'a':
				if constexpr (!is_contains_alpha<Pixel>)
				{
					throw std::format_error("Invalid format arg: "
											"pixel can't contain a channel.");
				}
				s(is_single_color<Pixel>? 1 : 3);
				break;

			case '}':
				return it;

			default:
				throw std::format_error("Invalid format arg for pixel");
			};
		}

		return it;
	}

	template<typename Out, typename CharP>
	FormatCnt<Out, CharP>::iterator
		format(Pixel pixel, FormatCnt<Out, CharP>& ctx) const
	{
		using namespace Nyaanwork;
		using namespace Nyaanwork::Asset::Pixels;

		Str str;

		auto i = this->i;
		if (i == 0)
			i = static_cast<u8>(pixel.channels)+1;

		auto ch = [&pixel](u8 i) -> u8
		{
			if constexpr (is_single_color<Pixel>)
			{
				if (i == 0)
					return pixel.r;
			}
			else
			{
				switch (i)
				{
				case 0:
					return pixel.r;
				case 1:
					return pixel.g;
				case 2:
					return pixel.b;

				default:
					break;
				}
			}

			if constexpr (is_contains_alpha<Pixel>)
				return pixel.a;

			return pixel.r;
		};

		switch (type)
		{
		case 'h':
			{
				switch(i)
				{
				case 1:
					str = std::format("{:0>2x}", ch(id[0]));
					break;
				case 2:
					{
						RA ra(ch(id[0]), ch(id[1]));
						str = std::format("{:0>4x}", ra.hex());
					}
					break;
				case 3:
					{
						RGB rgb(ch(id[0]), ch(id[1]), ch(id[2]));
						str = std::format("{:0>6x}", rgb.hex());
					}
					break;
				case 4:
					{
						RGBA rgba(ch(id[0]), ch(id[1]),
								  ch(id[2]), ch(id[3]));
						str = std::format("{:0>8x}", rgba.hex());
					}
					break;
				}
			}
			break;
		case 'f':
			str = get_pixel_fd_format<f32>(ch, i, spliter, id);
			break;
		case 'd':
			str = get_pixel_fd_format<u8>(ch, i, spliter, id);
			break;
		}

		auto toupper = [](char c) -> unsigned char { return std::toupper(c); };
		auto tolower = [](char c) -> unsigned char { return std::tolower(c); };
		std::ranges::transform(str, str.begin(), capitalize? toupper : tolower);

		if (quoted)
		{
			char f, s;
			switch (type)
			{
			case 'h':
				f = '#';
				s = '\0';
				break;
			case 'f':
			case 'd':
				f = '{';
				s = '}';
				break;
			}

			str = std::format("{}{}{}", f, std::move(str), s);
		}

		return std::ranges::copy(str, ctx.out()).out;
	}

	bool quoted = true;
	Nyaanwork::u8 i = 0;
	Nyaanwork::u8 id[4] = {};
	bool capitalize = true;
	char spliter = ',';
	char type = 'h';
};

export namespace Nyaanwork::Asset::Pixels
{
	template<typename Char, is_pixel Pixel>
	auto operator<<(std::basic_ostream<Char>& out, const Pixel& pixel)
		-> std::basic_ostream<Char>&
	{
		using S = Symbols<Char>;
		return out << S::open << std::hex << static_cast<u32>(pixel.hex()) << std::dec;
	}

	template<is_pixel Pixel>
	Str to_string(Pixel pixel)
		{ return std::format("{}", pixel); }
}

export template<Nyaanwork::Asset::Pixels::is_pixel Pixel>
struct std::hash<Pixel>
{
	static constexpr std::size_t operator()(Pixel pixel)
	{
		size_t hex = pixel.hex();
		size_t channels = static_cast<size_t>(Pixel::channels) + 1;
		return hash<size_t>{}(hex | (channels << 32));
	}
};
