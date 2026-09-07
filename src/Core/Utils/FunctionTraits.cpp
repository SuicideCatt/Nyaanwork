module;

#include <concepts>
#include <tuple>

export module Nyaanwork.Core.Utils.FunctionTraits;

import Nyaanwork.Core.Types;

namespace Nyaanwork::Utils
{
	template<typename... _Args>
	struct GetArgs
	{
		using Args = std::tuple<_Args...>;
		static constexpr usize args_count = sizeof...(_Args);

		template<usize i>
		using GetArg = std::tuple_element_t<i, Args>;

		using FirstArg = GetArg<0>;
		using LastArg = GetArg<args_count - 1>;
	};

	template<>
	struct GetArgs<>
	{
		using Args = std::tuple<>;
		static constexpr usize args_count = 0;

		template<usize i>
		using GetArg = void;

		using FirstArg = GetArg<0>;
		using LastArg = GetArg<0>;
	};

	template<typename Signature>
	struct FunctionTraitsT;

	template<typename Res, typename... Args>
	struct FunctionTraitsT<Res(*)(Args...)> : GetArgs<Args...>
	{
		using Ptr = Res(*)(Args...);

		using Result = Res;

		static constexpr bool is_noexcept = false;
	};

	template<typename Res, typename... Args>
	struct FunctionTraitsT<Res(*)(Args...) noexcept>
		: FunctionTraitsT<Res(*)(Args...)>
	{
		using Ptr = Res(*)(Args...) noexcept;

		static constexpr bool is_noexcept = true;
	};

	template<bool is_noexept, typename Res, typename... Args>
	struct GetFunctionTraitsT;

	template<typename Res, typename... Args>
	struct GetFunctionTraitsT<false, Res, Args...>
		: FunctionTraitsT<Res(*)(Args...)> {};

	template<typename Res, typename... Args>
	struct GetFunctionTraitsT<true, Res, Args...>
		: FunctionTraitsT<Res(*)(Args...) noexcept> {};

	template<typename Signature>
	struct MethodTraitsT;

	template<typename _Owner, typename Res, typename... Args>
	struct MethodTraitsT<Res(_Owner::*)(Args...)> : GetArgs<Args...>
	{
		using Owner = _Owner;
		using Ptr = Res(Owner::*)(Args...);

		using Result = Res;

		static constexpr bool is_const = false;
		static constexpr bool is_noexcept = false;
	};

	template<typename Owner, typename Res, typename... Args>
	struct MethodTraitsT<Res(Owner::*)(Args...) const>
		: MethodTraitsT<Res(Owner::*)(Args...)>
	{
		using Ptr = Res(Owner::*)(Args...) const;

		static constexpr bool is_const = true;
	};

	template<typename Owner, typename Res, typename... Args>
	struct MethodTraitsT<Res(Owner::*)(Args...) noexcept>
		: MethodTraitsT<Res(Owner::*)(Args...)>
	{
		using Ptr = Res(Owner::*)(Args...) noexcept;

		static constexpr bool is_noexcept = true;
	};

	template<typename Owner, typename Res, typename... Args>
	struct MethodTraitsT<Res(Owner::*)(Args...) const noexcept>
		: MethodTraitsT<Res(Owner::*)(Args...)>
	{
		using Ptr = Res(Owner::*)(Args...) const noexcept;

		static constexpr bool is_const = true;
		static constexpr bool is_noexcept = true;
	};

	template<bool is_const, bool is_noexept,
			 typename Owner, typename Res, typename... Args>
	struct GetMethodTraitsT;

	template<typename Owner, typename Res, typename... Args>
	struct GetMethodTraitsT<false, false, Owner, Res, Args...>
		: MethodTraitsT<Res(Owner::*)(Args...)> {};

	template<typename Owner, typename Res, typename... Args>
	struct GetMethodTraitsT<true, false, Owner, Res, Args...>
		: MethodTraitsT<Res(Owner::*)(Args...) const> {};

	template<typename Owner, typename Res, typename... Args>
	struct GetMethodTraitsT<false, true, Owner, Res, Args...>
		: MethodTraitsT<Res(Owner::*)(Args...) noexcept> {};

	template<typename Owner, typename Res, typename... Args>
	struct GetMethodTraitsT<true, true, Owner, Res, Args...>
		: MethodTraitsT<Res(Owner::*)(Args...) const noexcept> {};
}

export namespace Nyaanwork::Utils
{
	template<auto function>
	using FunctionTraits = FunctionTraitsT<decltype(function)>;

	template<auto method>
	using MethodTraits = MethodTraitsT<decltype(method)>;

	template<typename Lambda>
	using LambdaTraits = MethodTraits<&Lambda::operator()>;

	template<bool is_noexept, typename Res, typename... Args>
	using GetFunctionTraits = GetFunctionTraitsT<is_noexept, Res, Args...>;

	template<bool is_noexept, typename Res, typename... Args>
	using GetFunctionPtr = GetFunctionTraits<is_noexept, Res, Args...>::Ptr;

	template<bool is_const, bool is_noexept,
			 typename Owner, typename Res, typename... Args>
	using GetMethodTraits = GetMethodTraitsT<is_const, is_noexept,
											 Owner, Res, Args...>;

	template<bool is_const, bool is_noexept,
			 typename Owner, typename Res, typename... Args>
	using GetMethodPtr = GetMethodTraits<is_const, is_noexept,
										 Owner, Res, Args...>::Ptr;

	template<typename Owner, auto method>
	concept is_method_pointer_from =
		std::same_as<Owner, typename MethodTraits<method>::Owner>;

	template<typename Owner, typename Method>
	concept is_method_pointer_type_from =
		std::same_as<Owner, typename MethodTraitsT<Method>::Owner>;
}

export namespace Nyaanwork
{
	using namespace Utils;
}
