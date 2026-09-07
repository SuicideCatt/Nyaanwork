module;

#include <Core/Defines.hpp>

#include <concepts>

export module Nyaanwork.Asset.Image:Pixel;

import Nyaanwork.Core.Types;

namespace Nyaanwork::Asset::Pixels
{
	constexpr u8 mid(auto... chs)
	{
		return static_cast<u8>((static_cast<u16>(chs) + ...) / sizeof...(chs));
	}
}

export namespace Nyaanwork::Asset::Pixels
{
	enum class Channels : u8
	{
		r, ra, rgb, rgba
	};

	struct R;
	struct RA;
	struct RGB;
	struct RGBA;

	struct R final
	{
		constexpr R(u8 r = 0) noexcept
			: r(r) {}

		constexpr R(RA ra) noexcept;
		constexpr R(RGB rgb) noexcept;
		constexpr R(RGBA rgba) noexcept;

		constexpr R(const R&) = default;
		constexpr R& operator=(const R&) = default;

		constexpr auto operator<=>(const R& oth) NYAAN_CNOEX = default;

		constexpr u8 hex() NYAAN_CNOEX
			{ return r; }

		constexpr explicit operator u8() NYAAN_CNOEX
			{ return r; }

		constexpr explicit operator u16() NYAAN_CNOEX
			{ return static_cast<u16>(r); }

		constexpr explicit operator u32() NYAAN_CNOEX
			{ return static_cast<u32>(r); }

		constexpr explicit operator u64() NYAAN_CNOEX
			{ return static_cast<u64>(r); }

		constexpr explicit operator u128() NYAAN_CNOEX
			{ return static_cast<u128>(r); }

		constexpr explicit operator f32() NYAAN_CNOEX
			{ return static_cast<f32>(r) / 255_f32; }

		constexpr explicit operator f64() NYAAN_CNOEX
			{ return static_cast<f64>(r) / 255_f64; }

		constexpr explicit operator f128() NYAAN_CNOEX
			{ return static_cast<f128>(r) / 255_f128; }

		static constexpr auto channels = Channels::r;
		u8 r = 0;
	};

	struct RA
	{
		constexpr RA(i32 hex = 0x00FF) noexcept
			: RA((hex & 0xFF00) >> (8 * 1), (hex & 0x00FF) >> (8 * 0)) {}

		constexpr RA(u8 r, u8 a) noexcept
			: r(r), a(a) {}

		constexpr RA(R r, u8 a = 255) noexcept;
		constexpr RA(RGB rgb, u8 a = 255) noexcept;
		constexpr RA(RGBA rgba) noexcept;

		constexpr RA(const RA&) = default;
		constexpr RA& operator=(const RA&) = default;

		constexpr auto operator<=>(const RA& oth) NYAAN_CNOEX = default;

		constexpr u16 hex() NYAAN_CNOEX
		{
			return (
				(static_cast<u16>(r) << (8 * 1))
				| (static_cast<u16>(a) << (8 * 0))
			);
		}

		constexpr explicit operator vec2<u8>() NYAAN_CNOEX
			{ return vec2<u8>(r, a); }

		constexpr explicit operator vec2<u16>() NYAAN_CNOEX
			{ return vec2<u16>(r, a); }

		constexpr explicit operator vec2<u32>() NYAAN_CNOEX
			{ return vec2<u32>(r, a); }

		constexpr explicit operator vec2<u64>() NYAAN_CNOEX
			{ return vec2<u64>(r, a); }

		constexpr explicit operator vec2<u128>() NYAAN_CNOEX
			{ return vec2<u128>(r, a); }

		constexpr explicit operator vec2<f32>() NYAAN_CNOEX
			{ return vec2<f32>(r, a) / 255_f32; }

		constexpr explicit operator vec2<f64>() NYAAN_CNOEX
			{ return vec2<f64>(r, a) / 255_f64; }

		constexpr explicit operator vec2<f128>() NYAAN_CNOEX
			{ return vec2<f128>(r, a) / 255_f128; }

		static constexpr auto channels = Channels::ra;
		u8 r = 0, a = 255;
	};

	struct RGB final
	{
		constexpr RGB(i32 hex = 0x000000) noexcept
			: RGB((hex & 0xFF0000) >> (8 * 2),
				  (hex & 0x00FF00) >> (8 * 1),
				  (hex & 0x0000FF) >> (8 * 0)) {}

		constexpr RGB(u8 r, u8 g, u8 b) noexcept
			: r(r), g(g), b(b) {}

		constexpr RGB(R r) noexcept;
		constexpr RGB(RA ra) noexcept;
		constexpr RGB(RGBA rgba) noexcept;

		constexpr u32 hex() NYAAN_CNOEX
		{
			return (
				(static_cast<u32>(r) << (8 * 2))
				| (static_cast<u32>(g) << (8 * 1))
				| (static_cast<u32>(b) << (8 * 0))
			);
		}

		constexpr explicit operator vec3<u8>() NYAAN_CNOEX
			{ return vec3<u8>(r, g, b); }

		constexpr explicit operator vec3<u16>() NYAAN_CNOEX
			{ return vec3<u16>(r, g, b); }

		constexpr explicit operator vec3<u32>() NYAAN_CNOEX
			{ return vec3<u32>(r, g, b); }

		constexpr explicit operator vec3<u64>() NYAAN_CNOEX
			{ return vec3<u64>(r, g, b); }

		constexpr explicit operator vec3<u128>() NYAAN_CNOEX
			{ return vec3<u128>(r, g, b); }

		constexpr explicit operator vec3<f32>() NYAAN_CNOEX
			{ return vec3<f32>(r, g, b) / 255_f32; }

		constexpr explicit operator vec3<f64>() NYAAN_CNOEX
			{ return vec3<f64>(r, g, b) / 255_f64; }

		constexpr explicit operator vec3<f128>() NYAAN_CNOEX
			{ return vec3<f128>(r, g, b) / 255_f128; }

		static constexpr auto channels = Channels::rgb;
		u8 r = 0, g = 0, b = 0;
	};

	struct RGBA final
	{
		constexpr RGBA(i32 hex = 0x000000FF) noexcept
			: RGBA((hex & 0xFF000000) >> (8 * 3),
				   (hex & 0x00FF0000) >> (8 * 2),
				   (hex & 0x0000FF00) >> (8 * 1),
				   (hex & 0x000000FF) >> (8 * 0)) {}

		constexpr RGBA(u8 r, u8 g, u8 b, u8 a) noexcept
			: r(r), g(g), b(b), a(a) {}

		constexpr RGBA(R r, u8 a = 255) noexcept;
		constexpr RGBA(RA ra) noexcept;
		constexpr RGBA(RGB rgb, u8 a = 255) noexcept;

		constexpr u32 hex() NYAAN_CNOEX
		{
			return (
				(static_cast<u32>(r) << (8 * 3))
				| (static_cast<u32>(g) << (8 * 2))
				| (static_cast<u32>(b) << (8 * 1))
				| (static_cast<u32>(a) << (8 * 0))
			);
		}

		constexpr explicit operator vec4<u8>() NYAAN_CNOEX
			{ return vec4<u8>(r, g, b, a); }

		constexpr explicit operator vec4<u16>() NYAAN_CNOEX
			{ return vec4<u16>(r, g, b, a); }

		constexpr explicit operator vec4<u32>() NYAAN_CNOEX
			{ return vec4<u32>(r, g, b, a); }

		constexpr explicit operator vec4<u64>() NYAAN_CNOEX
			{ return vec4<u64>(r, g, b, a); }

		constexpr explicit operator vec4<u128>() NYAAN_CNOEX
			{ return vec4<u128>(r, g, b, a); }

		constexpr explicit operator vec4<f32>() NYAAN_CNOEX
			{ return vec4<f32>(r, g, b, a) / 255_f32; }

		constexpr explicit operator vec4<f64>() NYAAN_CNOEX
			{ return vec4<f64>(r, g, b, a) / 255_f64; }

		constexpr explicit operator vec4<f128>() NYAAN_CNOEX
			{ return vec4<f128>(r, g, b, a) / 255_f128; }

		static constexpr auto channels = Channels::rgba;
		u8 r = 0, g = 0, b = 0, a = 255;
	};

	constexpr R::R(RA ra) noexcept
		: R(ra.r * (static_cast<f64>(ra.a) / 255_f64)) {}

	constexpr R::R(RGB rgb) noexcept
		: R(mid(rgb.r, rgb.g, rgb.b)) {}

	constexpr R::R(RGBA rgba) noexcept
		: R(RA(rgba)) {}

	constexpr RA::RA(R r, u8 a) noexcept
		: RA(r.r, a) {}

	constexpr RA::RA(RGB rgb, u8 a) noexcept
		: RA(R(rgb), a) {}

	constexpr RA::RA(RGBA rgba) noexcept
		: RA(mid(rgba.r, rgba.g, rgba.b), rgba.a) {}

	constexpr RGB::RGB(R r) noexcept
		: RGB(r.r, r.r, r.r) {}

	constexpr RGB::RGB(RA ra) noexcept
		: RGB(R(ra)) {}

	constexpr RGB::RGB(RGBA rgba) noexcept
		: RGB([&rgba]
				{
					auto a = static_cast<f64>(rgba.a) / 255_f64;
					rgba.r *= a;
					rgba.g *= a;
					rgba.b *= a;
					return rgba.r;
				}(),
				rgba.g, rgba.b) {}

	constexpr RGBA::RGBA(R r, u8 a) noexcept
		: RGBA(r.r, r.r, r.r, a) {}

	constexpr RGBA::RGBA(RA ra) noexcept
		: RGBA(ra.r, ra.r, ra.r, ra.a) {}

	constexpr RGBA::RGBA(RGB rgb, u8 a) noexcept
		: RGBA(rgb.r, rgb.g, rgb.b, a) {}

	template<typename Pixel>
	concept is_pixel = std::same_as<Pixel, R> || std::same_as<Pixel, RA>
						|| std::same_as<Pixel, RGB> || std::same_as<Pixel, RGBA>;

	template<typename Pixel>
	concept is_single_color = std::same_as<Pixel, R> || std::same_as<Pixel, RA>;

	template<typename Pixel>
	concept is_multi_color = std::same_as<Pixel, RGB> || std::same_as<Pixel, RGBA>;

	template<typename Pixel>
	concept is_contains_alpha = std::same_as<Pixel, RA> || std::same_as<Pixel, RGBA>;
}
