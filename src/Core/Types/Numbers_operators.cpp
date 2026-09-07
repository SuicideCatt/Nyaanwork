export module Nyaanwork.Core.Types:Numbers_operators;
export import :Numbers;

export constexpr Nyaanwork::Types::i8 operator ""_i8(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::i8>(i); }

export constexpr Nyaanwork::Types::u8 operator ""_u8(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::u8>(i); }

export constexpr Nyaanwork::Types::i16 operator ""_i16(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::i16>(i); }

export constexpr Nyaanwork::Types::u16 operator ""_u16(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::u16>(i); }

export constexpr Nyaanwork::Types::i32 operator ""_i32(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::i32>(i); }

export constexpr Nyaanwork::Types::u32 operator ""_u32(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::u32>(i); }

export constexpr Nyaanwork::Types::i64 operator ""_i64(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::i64>(i); }

export constexpr Nyaanwork::Types::u64 operator ""_u64(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::u64>(i); }

export constexpr Nyaanwork::Types::i128 operator ""_i128(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::i128>(i); }

export constexpr Nyaanwork::Types::u128 operator ""_u128(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::u128>(i); }

export constexpr Nyaanwork::Types::isize operator ""_isize(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::isize>(i); }

export constexpr Nyaanwork::Types::usize operator ""_usize(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::usize>(i); }

export constexpr Nyaanwork::Types::f32 operator ""_f32(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::f32>(i); }

export constexpr Nyaanwork::Types::f64 operator ""_f64(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::f64>(i); }

export constexpr Nyaanwork::Types::f128 operator ""_f128(unsigned long long i)
	{ return static_cast<Nyaanwork::Types::f128>(i); }

export constexpr Nyaanwork::Types::f32 operator ""_f32(long double f)
	{ return static_cast<Nyaanwork::Types::f32>(f); }

export constexpr Nyaanwork::Types::f64 operator ""_f64(long double f)
	{ return static_cast<Nyaanwork::Types::f64>(f); }

export constexpr Nyaanwork::Types::f128 operator ""_f128(long double f)
	{ return static_cast<Nyaanwork::Types::f128>(f); }
