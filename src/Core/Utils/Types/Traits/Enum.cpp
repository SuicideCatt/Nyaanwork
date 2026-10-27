// i try rewrite vk::Flags

module;

#include <Core/Defines.hpp>

#include <concepts>

export module Nyaanwork.Core.Utils.Types:Traits.Enum;

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
}
