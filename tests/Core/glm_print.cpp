#include <iostream>
#include <print>

import Nyaanwork.Core.Types;

using namespace Nyaanwork;

template<auto l>
void test(vec<l, f32> vec)
{
	std::println("vec{}<f32> vec = {}", l, vec);

	#define FMT(fmt) std::println("{} -> " fmt, fmt, vec)

	FMT("{}");

	FMT("{:x}");
	FMT("{:y}");

	if constexpr(l >= 3)
		FMT("{:z}");
	if constexpr(l >= 4)
		FMT("{:w}");

	FMT("{:xy}");
	FMT("{:yx}");
	FMT("{:xx}");
	FMT("{:yy}");

	if constexpr(l >= 3)
	{
		FMT("{:xz}");
		FMT("{:zz}");
		FMT("{:zy}");
	}

	if constexpr(l >= 4)
	{
		FMT("{:xw}");
		FMT("{:wz}");
		FMT("{:ww}");
	}

	FMT("{:xyxy}");
	FMT("{:yxyx}");
	FMT("{:xxyy}");
	FMT("{:yyxx}");

	if constexpr(l >= 3)
	{
		FMT("{:xyz}");
		FMT("{:xyzz}");
		FMT("{:zzzz}");
		FMT("{:xzzy}");
		FMT("{:yzxz}");
	}

	if constexpr(l >= 4)
	{
		FMT("{:xyzw}");
		FMT("{:wyzz}");
		FMT("{:wwww}");
		FMT("{:zwzw}");
		FMT("{:xzwy}");
		FMT("{:yzxw}");
	}

	FMT("{:!x}");
	FMT("{:!y}");

	if constexpr(l >= 3)
		FMT("{:!zx}");
	if constexpr(l >= 4)
		FMT("{:!yw}");

	FMT("{:! xx}");
	FMT("{:! yy}");

	if constexpr(l >= 3)
		FMT("{:! zx}");
	if constexpr(l >= 4)
		FMT("{:! zw}");

	FMT("{:!,xx}");
	FMT("{:!,yy}");

	if constexpr(l >= 3)
		FMT("{:!,zx}");
	if constexpr(l >= 4)
		FMT("{:!,zw}");

	FMT("{:,xx}");
	FMT("{:,yy}");

	if constexpr(l >= 3)
		FMT("{:,zx}");
	if constexpr(l >= 4)
		FMT("{:,zw}");

	FMT("{: xx}");
	FMT("{: yy}");

	if constexpr(l >= 3)
		FMT("{: zx}");
	if constexpr(l >= 4)
		FMT("{: zw}");

	std::println();
}

int main(int argc, char** argv)
{
	test(vec2<f32>(1, 2));
	test(vec3<f32>(1, 2, 3));
	test(vec4<f32>(1, 2, 3, 4));

	std::cout << "iostream: " << vec2<f32>(1, 2) << '\n';
	std::cout << "iostream: " << vec3<f32>(1, 2, 3) << '\n';
	std::cout << "iostream: " << vec4<f32>(1, 2, 3, 4) << '\n';

	std::cout << "to_stirng: " << to_string(vec4<f32>(1, 2, 3, 4)) << '\n';

	return 0;
}
