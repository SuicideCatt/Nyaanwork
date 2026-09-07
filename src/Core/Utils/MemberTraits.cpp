module;

#include <concepts>

export module Nyaanwork.Core.Utils.MemberTraits;

import Nyaanwork.Core.Types;

namespace Nyaanwork::Utils
{
	template<typename Pointer>
	struct MemberTraitsT;

	template<typename _Type, typename _Owner>
	struct MemberTraitsT<_Type _Owner::*>
	{
		using Type = _Type;
		using Owner = _Owner;

		using Ptr = _Type _Owner::*;

		static constexpr bool is_const = false;
	};

	template<typename _Type, typename _Owner>
	struct MemberTraitsT<const _Type _Owner::*> : MemberTraitsT<_Type _Owner::*>
	{
		using Type = const _Type;

		using Ptr = Type _Owner::*;

		static constexpr bool is_const = true;
	};

	template<bool is_const, typename Type, typename Owner>
	struct GetMemberTraitsT;

	template<typename Type, typename Owner>
	struct GetMemberTraitsT<false, Type, Owner>
		: MemberTraitsT<Type Owner::*> {};

	template<typename Type, typename Owner>
	struct GetMemberTraitsT<true, Type, Owner>
		: MemberTraitsT<const Type Owner::*> {};
}

export namespace Nyaanwork::Utils
{
	template<auto member>
	using MemberTraits = MemberTraitsT<decltype(member)>;

	template<bool is_const, typename Type, typename Owner>
	using GetMemberTraits = GetMemberTraitsT<is_const, Type, Owner>;

	template<bool is_const, typename Type, typename Owner>
	using GetMemberPtr = typename GetMemberTraitsT<is_const, Type, Owner>::Ptr;

	template<typename Owner, auto member>
	concept is_member_pointer_from =
		std::same_as<Owner, typename MemberTraits<member>::Owner>;

	template<typename Owner, typename Member>
	concept is_member_pointer_type_from =
		std::same_as<Owner, typename MemberTraitsT<Member>::Owner>;
}

export namespace Nyaanwork
{
	using namespace Utils;
}
