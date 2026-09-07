// i try rewrite vk::Flags

module;

#include <Core/Defines.hpp>

#include <compare>
#include <concepts>

export module Nyaanwork.Core.Utils.Flags;

export namespace Nyaanwork::Utils
{
	template<typename Type>
	concept is_enum = std::is_enum_v<Type>;

	template<typename Type>
	concept is_scoped_enum = std::is_scoped_enum_v<Type>;

	template<typename Type, typename MaskType>
	concept is_scoped_enum_with_mask =
		is_scoped_enum<Type> && std::same_as<MaskType, std::underlying_type_t<Type>>;

	template<is_enum _Type>
	struct EnumTraits
	{
		using Type = _Type;
		using MaskType = std::underlying_type_t<_Type>;

		constexpr static MaskType cast(Type e) noexcept
			{ return static_cast<MaskType>(e); }

		constexpr static Type cast(MaskType e) noexcept
			{ return static_cast<Type>(e); }

		static MaskType* cast_ptr(Type* e) noexcept
			{ return reinterpret_cast<MaskType*>(&e); }

		static Type* cast_ptr(MaskType* e) noexcept
			{ return reinterpret_cast<Type*>(&e); }

		static const MaskType* cast_cptr(const Type* e) noexcept
			{ return reinterpret_cast<const MaskType*>(&e); }

		static const Type* cast_cptr(const MaskType* e) noexcept
			{ return reinterpret_cast<const Type*>(&e); }

		static MaskType& cast_ref(Type& e) noexcept
			{ return reinterpret_cast<MaskType&>(e); }

		static Type& cast_ref(MaskType& e) noexcept
			{ return reinterpret_cast<Type&>(e); }

		static const MaskType& cast_cref(const Type& e) noexcept
			{ return reinterpret_cast<const MaskType&>(e); }

		static const Type& cast_cref(const MaskType& e) noexcept
			{ return reinterpret_cast<const Type&>(e); }
	};

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
