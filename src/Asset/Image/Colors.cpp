module;

#include <concepts>
#include <iterator>

export module Nyaanwork.Asset.Image:Colors;
import :Pixel;

import Nyaanwork.Core.Types;

export namespace Nyaanwork::Asset::ITU_R
{
	constexpr vec3<f64> r601 = {0.299, 0.587, 0.114};
	constexpr vec3<f64> r709 = {0.2126, 0.7152, 0.0722};
	constexpr vec3<f64> r2020 = {0.2627, 0.678, 0.0593};

	constexpr vec3<f64> rdefault = r2020;
}

namespace Nyaanwork::Asset
{
	template<Pixels::is_pixel Pixel>
	struct BaseColors;

	template<typename Pixel>
	requires (Pixels::is_single_color<Pixel>)
	struct BaseColors<Pixel>
	{
		static constexpr vec3<f64> convert_weights = ITU_R::rdefault;

		static constexpr Pixel black = Pixels::R(0);
		static constexpr Pixel white = Pixels::R(255);

		static constexpr Pixel dark_gray = Pixels::R(0x70);
		static constexpr Pixel light_gray = Pixels::R(0xD3);

		static constexpr Pixel null = Pixel(0);
	};

	template<typename Pixel>
	requires (Pixels::is_multi_color<Pixel>)
	struct BaseColors<Pixel>
	{
		static constexpr vec3<f64> convert_weights = ITU_R::rdefault;

		static constexpr Pixel black = Pixels::RGB(0x000000);
		static constexpr Pixel white = Pixels::RGB(0xFFFFFF);

		static constexpr Pixel dark_gray = Pixels::RGB(0x707070);
		static constexpr Pixel light_gray = Pixels::RGB(0xD3D3D3);

		static constexpr Pixel red = Pixels::RGB(0xFF0000);
		static constexpr Pixel green = Pixels::RGB(0x00FF00);
		static constexpr Pixel blue = Pixels::RGB(0x0000FF);

		static constexpr Pixel yellow = Pixels::RGB(0xFFFF00);
		static constexpr Pixel magenta = Pixels::RGB(0xFF00FF);
		static constexpr Pixel cyan = Pixels::RGB(0x00FFFF);

		static constexpr Pixel light_red = Pixels::RGB(0xFF7272);
		static constexpr Pixel light_green = Pixels::RGB(0x72FF72);
		static constexpr Pixel light_blue = Pixels::RGB(0x7272FF);

		static constexpr Pixel light_yellow = Pixels::RGB(0xFFFF72);
		static constexpr Pixel light_magenta = Pixels::RGB(0xFF72FF);
		static constexpr Pixel light_cyan = Pixels::RGB(0x72FFFF);

		static constexpr Pixel dark_red = Pixels::RGB(0x990000);
		static constexpr Pixel dark_green = Pixels::RGB(0x009900);
		static constexpr Pixel dark_blue = Pixels::RGB(0x000099);

		static constexpr Pixel dark_yellow = Pixels::RGB(0x999900);
		static constexpr Pixel dark_magenta = Pixels::RGB(0x990099);
		static constexpr Pixel dark_cyan = Pixels::RGB(0x009999);

		static constexpr Pixel null = Pixel(0);

		template<Pixels::is_pixel _Pixel>
		constexpr static Pixel convert(_Pixel pixel,
									   vec3<f64> weights = convert_weights) noexcept
			{ return pixel; }

		template<typename Input, typename Output,
				 typename OutputVT = std::iterator_traits<Output>::value_type>
		requires (std::same_as<OutputVT, Pixel>)
		constexpr static void
			convert(const Input i_begin, const Input i_end, Output o_begin,
					vec3<f64> weights = convert_weights) noexcept
		{
			for (auto itr = i_begin; itr != i_end; ++itr)
				*(o_begin++) = convert(*itr, weights);
		}
	};
}

export namespace Nyaanwork::Asset
{
	template<Pixels::is_pixel Pixel>
	struct Colors;

	template<>
	struct Colors<Pixels::R> : BaseColors<Pixels::R>
	{
		using Pixel = Pixels::R;

		template<Pixels::is_pixel _Pixel>
		requires (Pixels::is_single_color<_Pixel>)
		constexpr static Pixel convert(_Pixel pixel,
									   vec3<f64> weights = convert_weights) noexcept
			{ return pixel; }

		template<Pixels::is_pixel _Pixel>
		requires (!Pixels::is_single_color<_Pixel>)
		constexpr static Pixel convert(_Pixel pixel,
									   vec3<f64> weights = convert_weights) noexcept
		{
			const Pixels::RGB& rgb = pixel;
			auto [r, g, b] = vec3<f64>(rgb.r, rgb.g, rgb.b) * weights;
			return static_cast<u8>((r + g + b) / 3_f64);
		}

		template<typename Input, typename Output,
				 typename OutputVT = std::iterator_traits<Output>::value_type>
		requires (std::same_as<OutputVT, Pixel>)
		constexpr static void
			convert(const Input i_begin, const Input i_end, Output o_begin,
					vec3<f64> weights = convert_weights) noexcept
		{
			for (auto itr = i_begin; itr != i_end; ++itr)
				*(o_begin++) = convert(*itr, weights);
		}
	};

	template<>
	struct Colors<Pixels::RA> : BaseColors<Pixels::RA>
	{
		using Pixel = Pixels::RA;

		template<Pixels::is_pixel _Pixel>
		requires (Pixels::is_single_color<_Pixel>)
		constexpr static Pixel convert(_Pixel pixel,
									   vec3<f64> weights = convert_weights) noexcept
			{ return pixel; }

		template<Pixels::is_pixel _Pixel>
		requires (!Pixels::is_single_color<_Pixel>)
		constexpr static Pixel convert(_Pixel pixel,
									   vec3<f64> weights = convert_weights) noexcept
		{
			const Pixels::RGBA& rgba = pixel;
			Pixels::RGB rgb(rgba.r, rgba.g, rgba.b);
			return {
				Colors<Pixels::R>::convert(rgb, weights),
				rgba.a,
			};
		}

		template<typename Input, typename Output,
				 typename OutputVT = std::iterator_traits<Output>::value_type>
		requires (std::same_as<OutputVT, Pixel>)
		constexpr static void
			convert(const Input i_begin, const Input i_end, Output o_begin,
					vec3<f64> weights = convert_weights) noexcept
		{
			for (auto itr = i_begin; itr != i_end; ++itr)
				*(o_begin++) = convert(*itr, weights);
		}
	};

	template<>
	struct Colors<Pixels::RGB> : BaseColors<Pixels::RGB>
	{
		using Pixel = Pixels::RGB;
	};

	template<>
	struct Colors<Pixels::RGBA> : BaseColors<Pixels::RGBA>
	{
		using Pixel = Pixels::RGBA;
	};
}
