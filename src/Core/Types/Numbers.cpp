module;

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>

#ifndef __SIZEOF_INT128__
	#error "No int128?"
#endif

export module Nyaanwork.Core.Types:Numbers;

// Numbers
export namespace Nyaanwork::Types
{
	using i8 = int8_t;
	using u8 = uint8_t;

	using i16 = int16_t;
	using u16 = uint16_t;

	using i32 = int32_t;
	using u32 = uint32_t;

	using i64 = int64_t;
	using u64 = uint64_t;

	using i128 = __int128_t;
	using u128 = __uint128_t;

	using isize = std::ptrdiff_t;
	using usize = size_t;

	template<typename Type>
	concept is_integral = std::integral<Type>;

	using f32 = float;
	using f64 = double;
	using f128 = long double;

	template<typename Type>
	concept is_floating = std::floating_point<Type>;

	template<typename Type>
	concept is_arithmetic = std::is_arithmetic_v<Type>;

	template<is_arithmetic Type>
	using limits = std::numeric_limits<Type>;
}
