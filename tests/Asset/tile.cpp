#include <print>

import Nyaanwork.Asset.Image;
import Nyaanwork.Asset.Tile;
import Nyaanwork.Core.Types;

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
		std::print(" ");
		for (u16 x = 0; x < res.x; ++x)
		{
			auto& pixel = image.at(x, y);

			auto& [r, g, b] = pixel;
			std::print("\x1b[0;38;2;{};{};{}m", r, g, b);
			std::print(" {}", pixel);
			std::print("\x1b[0m");
		}
		std::println();
	}
	std::println();
}

int main(int argc, char** argv)
{
	#define print_image(var) print_image(#var, var)

	Image img_orig({9, 6}, {
		RGB(0xFF0000), RGB(0xFF0001), RGB(0xFF0002), RGB(0x00FF00), RGB(0x00FF01), RGB(0x00FF02), RGB(0x0000FF), RGB(0x0100FF), RGB(0x0200FF),
		RGB(0xFF0003), RGB(0xFF0004), RGB(0xFF0005), RGB(0x00FF03), RGB(0x00FF04), RGB(0x00FF05), RGB(0x0300FF), RGB(0x0400FF), RGB(0x0500FF),
		RGB(0xFF0006), RGB(0xFF0007), RGB(0xFF0008), RGB(0x00FF06), RGB(0x00FF07), RGB(0x00FF08), RGB(0x0600FF), RGB(0x0700FF), RGB(0x0800FF),
		RGB(0xFFFFF0), RGB(0xFFFFF1), RGB(0xFFFFF2), RGB(0x000000), RGB(0x000001), RGB(0x000002), RGB(0xF0F0F0), RGB(0xF1F1F1), RGB(0xF2F2F2),
		RGB(0xFFFFF3), RGB(0xFFFFF4), RGB(0xFFFFF5), RGB(0x000003), RGB(0x000004), RGB(0x000005), RGB(0xF3F3F3), RGB(0xF4F4F4), RGB(0xF5F5F5),
		RGB(0xFFFFF6), RGB(0xFFFFF7), RGB(0xFFFFF8), RGB(0x000006), RGB(0x000007), RGB(0x000008), RGB(0xF6F6F6), RGB(0xF7F7F7), RGB(0xF8F8F8),
	});

	Image img = img_orig;
	TileSet<decltype(img)::Pixel> set(img, {3, 3});

	// Tile{{1}}: ++itr
	std::swap(set[2], set[0]);
	std::swap(set[4], set[5]);

	print_image(img_orig);
	print_image(img);

	std::println("Tile{{1}}: itr + i");
	auto& tile = set[1];
	for (usize i = 0; i < tile.size(); ++i)
	{
		auto& pixel = tile[i];

		auto& [r, g, b] = pixel;
		std::print("  \x1b[0;38;2;{};{};{}m", r, g, b);
		std::print("{}", pixel);
		std::println("\x1b[0m");
	}
	std::println();

	std::println("Tile{{1}}: itr - i");
	for (usize i = 0; i < tile.size(); ++i)
	{
		auto f = tile.size()-i;
		auto& pixel = *(tile.end() - f);

		auto& [r, g, b] = pixel;
		std::print("  \x1b[0;38;2;{};{};{}m", r, g, b);
		std::print("{}", pixel);
		std::println("\x1b[0m");
	}
	std::println();

	std::println("Tile{{1}}: --itr (reverse test)");
	for (auto itr = tile.rbegin(); itr != tile.rend(); ++itr)
	{
		auto& pixel = *itr;

		auto& [r, g, b] = pixel;
		std::print("  \x1b[0;38;2;{};{};{}m", r, g, b);
		std::print("{}", pixel);
		std::println("\x1b[0m");
	}
	std::println();

	auto img_from_tile = tile.create_image();
	print_image(img_from_tile);

	std::println("reverse img_from_tile print");
	for (auto itr = img_from_tile.rbegin(); itr != img_from_tile.rend(); ++itr)
	{
		auto& pixel = *itr;

		auto& [r, g, b] = pixel;
		std::print("  \x1b[0;38;2;{};{};{}m", r, g, b);
		std::print("{}", pixel);
		std::println("\x1b[0m");
	}

	return 0;
}
