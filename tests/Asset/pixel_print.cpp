#include <iostream>
#include <print>

import Nyaanwork.Core.Types;
import Nyaanwork.Asset.Image;

using namespace Nyaanwork;
using namespace Nyaanwork::Asset;
using namespace Nyaanwork::Asset::Pixels;

template<typename NA, typename A>
requires (!is_contains_alpha<NA> && is_contains_alpha<A>
		&& ((is_single_color<NA> && is_single_color<A>)
			|| (is_multi_color<NA> && is_multi_color<A>)))
void test_convert(const NA& na, const A& a)
{
	using ONA = std::conditional_t<is_single_color<NA>, RGB, R>;
	using OA = std::conditional_t<is_single_color<A>, RGBA, RA>;

	{
		std::println("{:l}, {:l}", na, a);
		std::println("convert to no alpha");
		auto from_na = Colors<ONA>::convert(na);
		auto from_a = Colors<ONA>::convert(a);
		std::println("  {:l}", from_na);
		std::println("  {:l}", from_a);
		std::println("  convert back");
		std::println("    {:l}", Colors<NA>::convert(from_na));
		std::println("    {:l}", Colors<NA>::convert(from_a));
	}

	{
		std::println("{:l}, {:l}", na, a);
		std::println("convert with alpha", a);
		auto from_na = Colors<OA>::convert(na);
		auto from_a = Colors<OA>::convert(a);
		std::println("  {:l}", from_na);
		std::println("  {:l}", from_a);
		std::println("  convert back");
		std::println("    {:l}", Colors<A>::convert(from_na));
		std::println("    {:l}", Colors<A>::convert(from_a));
	}
	std::println();
}

int main(int argc, char** argv)
{
	#define FMT(fmt, pixel) std::println("{} -> " fmt, fmt, pixel)

	static constexpr R r = 0xF0;
	std::println("R r = {0} -> (#{0:0>2x})", r.hex());

	FMT("{}", r);
	FMT("{:r}", r);
	FMT("{:rr}", r);
	FMT("{:rrr}", r);
	FMT("{:rrrr}", r);

	FMT("{:!r}", r);
	FMT("{:!rr}", r);
	FMT("{:!rrr}", r);
	FMT("{:!rrrr}", r);

	FMT("{:dr}", r);
	FMT("{:drr}", r);
	FMT("{:drrr}", r);
	FMT("{:drrrr}", r);

	FMT("{:!dr}", r);
	FMT("{:!drr}", r);
	FMT("{:!drrr}", r);
	FMT("{:!drrrr}", r);

	FMT("{:,dr}", r);
	FMT("{:,drr}", r);
	FMT("{:,drrr}", r);
	FMT("{:,drrrr}", r);

	FMT("{:!,dr}", r);
	FMT("{:!,drr}", r);
	FMT("{:!,drrr}", r);
	FMT("{:!,drrrr}", r);

	FMT("{: dr}", r);
	FMT("{: drr}", r);
	FMT("{: drrr}", r);
	FMT("{: drrrr}", r);

	FMT("{:! dr}", r);
	FMT("{:! drr}", r);
	FMT("{:! drrr}", r);
	FMT("{:! drrrr}", r);

	FMT("{:fr}", r);
	FMT("{:frr}", r);
	FMT("{:frrr}", r);
	FMT("{:frrrr}", r);

	FMT("{:!fr}", r);
	FMT("{:!frr}", r);
	FMT("{:!frrr}", r);
	FMT("{:!frrrr}", r);

	FMT("{:,fr}", r);
	FMT("{:,frr}", r);
	FMT("{:,frrr}", r);
	FMT("{:,frrrr}", r);

	FMT("{:!,fr}", r);
	FMT("{:!,frr}", r);
	FMT("{:!,frrr}", r);
	FMT("{:!,frrrr}", r);

	FMT("{: fr}", r);
	FMT("{: frr}", r);
	FMT("{: frrr}", r);
	FMT("{: frrrr}", r);

	FMT("{:! fr}", r);
	FMT("{:! frr}", r);
	FMT("{:! frrr}", r);
	FMT("{:! frrrr}", r);

	std::cout << "iostream: " << r << '\n';
	std::cout << "to_string: " << to_string(r) << '\n';

	std::println();

	static constexpr RA ra = 0xF0AA;
	std::println("RA ra = {0} -> (#{0:0>4x})", ra.hex());

	FMT("{}", ra);
	FMT("{:r}", ra);
	FMT("{:a}", ra);
	FMT("{:fr}", ra);
	FMT("{:fa}", ra);
	FMT("{:ra}", ra);
	FMT("{:ar}", ra);
	FMT("{:rr}", ra);
	FMT("{:,daa}", ra);
	FMT("{: fara}", ra);
	FMT("{:farar}", ra);
	FMT("{:drar}", ra);
	FMT("{:!rara}", ra);

	std::cout << "iostream: " << ra << '\n';
	std::cout << "to_string: " << to_string(ra) << '\n';

	std::println();

	static constexpr RGB rgb = 0xF0F1F2;
	std::println("RGB rgb = {0} -> (#{0:0>6x})", rgb.hex());

	FMT("{}", rgb);
	FMT("{:r}", rgb);
	FMT("{:g}", rgb);
	FMT("{:b}", rgb);
	FMT("{:fr}", rgb);
	FMT("{:fg}", rgb);
	FMT("{:fb}", rgb);
	FMT("{:rgb}", rgb);
	FMT("{:bgr}", rgb);
	FMT("{:rbr}", rgb);
	FMT("{:,rbg}", rgb);
	FMT("{: fgrb}", rgb);
	FMT("{:fbrgr}", rgb);
	FMT("{:dggg}", rgb);
	FMT("{:!bbrb}", rgb);

	std::cout << "iostream: " << rgb << '\n';
	std::cout << "to_string: " << to_string(rgb) << '\n';

	std::println();

	static constexpr RGBA rgba = 0xF0F1F2AA;
	std::println("RGBA rgba = {0} -> (#{0:0>8x})", rgba.hex());

	FMT("{}", rgba);
	FMT("{:r}", rgba);
	FMT("{:g}", rgba);
	FMT("{:b}", rgba);
	FMT("{:a}", rgba);
	FMT("{:fr}", rgba);
	FMT("{:fg}", rgba);
	FMT("{:fb}", rgba);
	FMT("{:fa}", rgba);
	FMT("{:rgba}", rgba);
	FMT("{:bgra}", rgba);
	FMT("{:arbr}", rgba);
	FMT("{:,arbg}", rgba);
	FMT("{:f agrb}", rgba);
	FMT("{:faagr}", rgba);
	FMT("{:dgagg}", rgba);
	FMT("{:!bbra}", rgba);

	std::cout << "iostream: " << rgba << '\n';
	std::cout << "to_string: " << to_string(rgba) << '\n';

	std::println();

	#undef FMT

	#define FMT(space, color) std::println("{} -> {}: {}", #space, #color, \
										   Colors<space>::color)

	FMT(R, black);
	FMT(R, white);
	FMT(R, dark_gray);
	FMT(R, light_gray);
	FMT(R, null);
	std::println();

	FMT(RA, black);
	FMT(RA, white);
	FMT(RA, dark_gray);
	FMT(RA, light_gray);
	FMT(RA, null);
	std::println();

	FMT(RGB, black);
	FMT(RGB, white);
	FMT(RGB, dark_gray);
	FMT(RGB, light_gray);
	FMT(RGB, red);
	FMT(RGB, green);
	FMT(RGB, blue);
	FMT(RGB, yellow);
	FMT(RGB, magenta);
	FMT(RGB, cyan);
	FMT(RGB, light_red);
	FMT(RGB, light_green);
	FMT(RGB, light_blue);
	FMT(RGB, light_yellow);
	FMT(RGB, light_magenta);
	FMT(RGB, light_cyan);
	FMT(RGB, dark_red);
	FMT(RGB, dark_green);
	FMT(RGB, dark_blue);
	FMT(RGB, dark_yellow);
	FMT(RGB, dark_magenta);
	FMT(RGB, dark_cyan);
	FMT(RGB, null);
	std::println();

	FMT(RGBA, black);
	FMT(RGBA, white);
	FMT(RGBA, dark_gray);
	FMT(RGBA, light_gray);
	FMT(RGBA, red);
	FMT(RGBA, green);
	FMT(RGBA, blue);
	FMT(RGBA, yellow);
	FMT(RGBA, magenta);
	FMT(RGBA, cyan);
	FMT(RGBA, light_red);
	FMT(RGBA, light_green);
	FMT(RGBA, light_blue);
	FMT(RGBA, light_yellow);
	FMT(RGBA, light_magenta);
	FMT(RGBA, light_cyan);
	FMT(RGBA, dark_red);
	FMT(RGBA, dark_green);
	FMT(RGBA, dark_blue);
	FMT(RGBA, dark_yellow);
	FMT(RGBA, dark_magenta);
	FMT(RGBA, dark_cyan);
	FMT(RGBA, null);
	std::println();

	#undef FMT

	test_convert(R(0xF0), RA(R(0xF0), 0x88));
	test_convert(RGB(0xF0F1F2), RGBA(RGB(0xF0F1F2), 0x88));

	static constexpr auto c1 = Colors<R>::convert(RA(0xFF88)); // 0x88
	static constexpr auto c2 = Colors<R>::convert(RGB(0xFFFFFF)); // 0x55
	static constexpr auto c3 = Colors<R>::convert(RGBA(0xFFFFFFFF)); // 0x55
	static constexpr auto c4 = Colors<R>::convert(RGBA(0xFFFFFF88)); // 0x2D

	static constexpr auto c5 = Colors<RA>::convert(R(0xFF)); // 0xFFFF
	static constexpr auto c6 = Colors<RA>::convert(RGB(0xFFFFFF)); // 0x55FF
	static constexpr auto c7 = Colors<RA>::convert(RGBA(0xFFFFFFFF)); // 0x55FF
	static constexpr auto c8 = Colors<RA>::convert(RGBA(0xFFFFFF88)); // 0x55FF

	return 0;
}
