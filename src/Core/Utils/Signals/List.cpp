module;

#include <concepts>

export module Nyaanwork.Core.Utils.Signals:List;

import Nyaanwork.Core.Utils.Flags;

namespace Nyaanwork::Utils
{
	template<is_scoped_enum Slots, typename _Front, typename _PopFront>
	struct ListNode
	{
		static_assert(std::same_as<Slots, typename _Front::Slots>,
					  "Slots enum type does not match");

		using PopFront = _PopFront; // next ListNode
		using Front = _Front; // SlotInfo
	};

	template<is_scoped_enum Slots, typename Front, typename... Others>
	struct List : ListNode<Slots, Front, List<Slots, Others...>> {};

	template<is_scoped_enum Slots, typename Front>
	struct List<Slots, Front> : ListNode<Slots, Front, void> {};

	// loop
	template<bool error, auto slot, typename List>
	struct Find : Find<error, slot, typename List::PopFront> {};

	// if requires pass loop ends
	template<bool error, auto slot, typename List>
	requires (List::Front::slot == slot)
	struct Find<error, slot, List>
	{
		using SlotInfo = List::Front;
	};

	template<auto slot>
	struct Find<true, slot, void>
	{
		static_assert(false, "Cannot find the slot");
	};

	// for IsUniqueList
	template<auto slot>
	struct Find<false, slot, void>
	{
		using SlotInfo = void;
	};

	// is like Find but have SingleSlotInfo logic
	template<is_scoped_enum Slots, Slots slot, typename SlotInfos, bool single>
	struct FindSlotInfo
	{
		using SlotInfo = Find<true, slot, SlotInfos>::SlotInfo;
	};

	template<is_scoped_enum Slots, Slots slot, typename SlotInfos>
	struct FindSlotInfo<Slots, slot, SlotInfos, true>
	{
		using SlotInfo = SlotInfos::Front;
	};

	template<auto slot, typename List>
	concept contain_slot_info =
		!std::is_void_v<typename Find<false, slot, List>::SlotInfo>;

	template<is_scoped_enum _Slots, typename Front, typename... Others>
	struct IsUniqueList
	{
		static constexpr auto slot = Front::SlotInfo::slot;
		static constexpr bool contain =
			contain_slot_info<slot, List<_Slots, Others...>>;

		static_assert(!contain, "Detect: duplicates in the SlotInfos list");

		using Next = IsUniqueList<_Slots, Others...>; // check Others

		// Get Slots enum type from Next, launch loop, just garant of work
		using Slots = typename Next::Slots;
	};

	template<is_scoped_enum _Slots, typename First>
	struct IsUniqueList<_Slots, First>
	{
		using Slots = _Slots;
	};
}
