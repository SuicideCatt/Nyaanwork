module;

export module Nyaanwork.Core.Utils.Signals:SlotInfo;
import :Callers;

import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.FunctionTraits;

export namespace Nyaanwork::Utils
{
	template<auto slot, bool is_noexcept, typename... Args>
	struct SlotInfo;

	// SlotInfo and SingleSlotInfo create and containt all meta-data
	template<bool is_noexcept, typename... Args>
	struct SingleSlotInfo
	{
		using Function = GetFunctionTraits<is_noexcept, void, Args...>;

		template<typename Owner>
		using Method = GetMethodTraits<false, is_noexcept, Owner, void, Args...>;

		struct Callers
		{
			using Base = BaseCaller<is_noexcept, Args...>;

			template<auto function>
			using Function = FunctionCaller<is_noexcept, function, Args...>;

			template<typename Owner, auto method>
			using Method = MethodCaller<is_noexcept, Owner, method, Args...>;

			template<typename _Functor>
			using Functor = FunctorCaller<is_noexcept, _Functor, Args...>;
		};

		static constexpr bool single = true;

		template<is_scoped_enum Slots>
		using MultiSlotInfo = SlotInfo<Slots{}, is_noexcept, Args...>;
	};

	template<auto slot_, bool is_noexcept, typename... Args>
	struct SlotInfo : SingleSlotInfo<is_noexcept, Args...>
	{
		using Slots = decltype(slot_);
		static constexpr Slots slot = slot_;

		static constexpr bool single = false;
	};
}
