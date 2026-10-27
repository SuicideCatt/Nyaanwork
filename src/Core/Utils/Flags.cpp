// i try rewrite vk::Flags

module;

#include <Core/Defines.hpp>

#include <compare>

export module Nyaanwork.Core.Utils.Flags;

import Nyaanwork.Core.Utils.Types;

export namespace Nyaanwork::Utils
{
	template<is_scoped_enum Type>
	struct FlagInfo : EnumTraits<Type>
	{
		using BitType = Type;
	};

	template<is_scoped_enum BitType>
	struct FlagTraits
	{
		static constexpr bool is_bit_type = false;
	};

	template<typename BitType>
	concept is_bit_type = FlagTraits<BitType>::is_bit_type;

	template<is_bit_type _BitType>
	class Flags
	{
	public:
		using Traits = FlagTraits<_BitType>;

		using BitType = Traits::BitType;
		using MaskType = Traits::MaskType;

		constexpr Flags() = default;

		constexpr Flags(_BitType bit) noexcept
			: m_mask(Traits::cast(bit)) {}

		constexpr explicit Flags(MaskType flags) noexcept
			: m_mask(flags) {}

		constexpr Flags(const Flags&) = default;
		constexpr Flags& operator=(const Flags&) = default;

		constexpr Flags operator&(const Flags& oth) NYAAN_CNOEX
			{ return Flags(m_mask & oth.m_mask); }

		constexpr Flags operator|(const Flags& oth) NYAAN_CNOEX
			{ return Flags(m_mask | oth.m_mask); }

		constexpr Flags operator^(const Flags& oth) NYAAN_CNOEX
			{ return Flags(m_mask ^ oth.m_mask); }

		constexpr Flags operator~() NYAAN_CNOEX
			{ return *this ^ Traits::all; }

		constexpr Flags& operator&=(const Flags& oth) noexcept
		{
			m_mask &= oth.m_mask;
			return *this;
		}

		constexpr Flags& operator|=(const Flags& oth) noexcept
		{
			m_mask |= oth.m_mask;
			return *this;
		}

		constexpr Flags& operator^=(const Flags& oth) noexcept
		{
			m_mask ^= oth.m_mask;
			return *this;
		}

		constexpr auto operator<=>(const Flags&) const noexcept = default;
		constexpr operator bool() NYAAN_CNOEX
			{ return m_mask; }

		constexpr operator MaskType() NYAAN_CNOEX
			{ return m_mask; }

	private:
		MaskType m_mask = MaskType{};
	};
}

export namespace Nyaanwork
{
	using namespace Utils;
}

export template<Nyaanwork::is_bit_type BitType>
constexpr Nyaanwork::Flags<BitType> operator&(BitType a, BitType b) noexcept
	{ return Nyaanwork::Flags(a) & b; }

export template<Nyaanwork::is_bit_type BitType>
constexpr Nyaanwork::Flags<BitType> operator|(BitType a, BitType b) noexcept
	{ return Nyaanwork::Flags(a) | b; }

export template<Nyaanwork::is_bit_type BitType>
constexpr Nyaanwork::Flags<BitType> operator^(BitType a, BitType b) noexcept
	{ return Nyaanwork::Flags(a) ^ b; }

export template<Nyaanwork::is_bit_type BitType>
constexpr Nyaanwork::Flags<BitType> operator~(BitType a) noexcept
	{ return ~Nyaanwork::Flags(a); }

export template<Nyaanwork::is_bit_type BitType>
constexpr Nyaanwork::Flags<BitType>
	operator&(BitType a, const Nyaanwork::Flags<BitType>& b) noexcept
	{ return Nyaanwork::Flags(a) & b; }

export template<Nyaanwork::is_bit_type BitType>
constexpr Nyaanwork::Flags<BitType>
	operator|(BitType a, const Nyaanwork::Flags<BitType>& b) noexcept
	{ return Nyaanwork::Flags(a) | b; }

export template<Nyaanwork::is_bit_type BitType>
constexpr Nyaanwork::Flags<BitType>
	operator^(BitType a, const Nyaanwork::Flags<BitType>& b) noexcept
	{ return Nyaanwork::Flags(a) ^ b; }
