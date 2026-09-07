#include <print>

import Nyaanwork.Core.Types;
import Nyaanwork.Asset.Image;

using namespace Nyaanwork;
using namespace Nyaanwork::Asset;
using namespace Nyaanwork::Asset::Pixels;

template<is_pixel Pixel>
void print_image(std::string_view name, const Image<Pixel>& image)
{
	auto res = image.resolution();
	std::println("{}: resolution: {}", name, res);
	for (u16 y = 0; y < res.y; ++y)
	{
		for (u16 x = 0; x < res.x; ++x)
		{
			std::print("{} ", image.at(x, y));
		}
		std::println();
	}
	std::println();
}

template<is_pixel Pixel>
void print_image_view(std::string_view name, ImageView<Pixel> image)
{
	auto res = image.resolution();
	std::println("{}: resolution: {}", name, res);
	for (u16 y = 0; y < res.y; ++y)
	{
		for (u16 x = 0; x < res.x; ++x)
		{
			std::print("{} ", image.at(x, y));
		}
		std::println();
	}
	std::println();
}

template<is_pixel Pixel>
void print_const_image_view(std::string_view name, ImageView<const Pixel> image)
{
	auto res = image.resolution();
	std::println("{}: resolution: {}", name, res);
	for (u16 y = 0; y < res.y; ++y)
	{
		for (u16 x = 0; x < res.x; ++x)
		{
			std::print("{} ", image.at(x, y));
		}
		std::println();
	}
	std::println();
}

int main(int argc, char** argv)
{
	#define print_image(var) print_image(#var, var)
	#define print_image_view(var) print_image_view(#var, var)
	#define print_const_image_view(var) print_const_image_view(#var, var)

	Image<R> r({2, 2}, {
		R(255), R(255),
		R(255), R(0)
	});

	Image<RA> ra({2, 2}, std::vector({
		RA(255, 255), RA(255, 168),
		RA(255, 84), RA(0, 0)
	}));

	Image<RGB> rgb({2, 3}, {
		RGB(0xFF0000), RGB(0x00FF00),
		RGB(0x0000FF), RGB(0xFFFFFF),
		RGB(0xF0F1F2), RGB(0x000000),
	});

	Image<RGBA> rgba({2, 3}, {
		RGBA(0xFF0000FF), RGBA(0x00FF00FF),
		RGBA(0x0000FFFF), RGBA(0xFFFFFFFF),
		RGBA(0xF0F1F2AA), RGBA(0x00000000),
	});

	print_image(r);
	print_image(ra);
	print_image(rgb);
	print_image(rgba);

	Image<R> r_from_rgba = rgba;
	Image<RA> ra_from_rgba = rgba;
	print_image(r_from_rgba);
	print_image(ra_from_rgba);

	Image<RGB>rgb_from_ra = ra;
	Image<RGBA> rgba_from_r = r;
	print_image(rgb_from_ra);
	print_image(rgba_from_r);

	RGB prgb;
	Image<RGB>::iterator itr(&prgb);

	ImageView<R> view;
	ImageView view_r(r);
	ImageView view_ra(ra);
	ImageView view_rgb(rgb);
	ImageView view_rgba(rgba);

	ImageView<const R> const_view;
	ImageView const_view_r = view_r.to_const();
	ImageView const_view_ra = view_ra.to_const();
	ImageView const_view_rgb = view_rgb.to_const();
	ImageView const_view_rgba = view_rgba.to_const();

	ImageView<const R> const_view_r_from_image = r;
	ImageView<const RA> const_view_ra_from_image = ra;
	ImageView<const RGB> const_view_rgb_from_image = rgb;
	ImageView<const RGBA> const_view_rgba_from_image = rgba;

	print_image_view(view);
	print_image_view(view_r);
	print_image_view(view_ra);
	print_image_view(view_rgb);
	print_image_view(view_rgba);

	print_const_image_view(const_view);
	print_const_image_view(const_view_r);
	print_const_image_view(const_view_ra);
	print_const_image_view(const_view_rgb);
	print_const_image_view(const_view_rgba);

	print_const_image_view(const_view_r_from_image);
	print_const_image_view(const_view_ra_from_image);
	print_const_image_view(const_view_rgb_from_image);
	print_const_image_view(const_view_rgba_from_image);

	return 0;
}
