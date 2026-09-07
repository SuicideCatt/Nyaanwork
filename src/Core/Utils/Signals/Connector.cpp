module;

#include <concepts>
#include <utility>

export module Nyaanwork.Core.Utils.Signals:Connector;
import :List;
import :SlotInfo;

import Nyaanwork.Core.Utils.ClearType;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.FunctionTraits;

namespace Nyaanwork::Utils
{
	// BaseSignalsConnector contain universal signal_connect methods
	template<is_scoped_enum _Slots, bool single, typename CallersContainer, typename... Infos>
	class BaseSignalsConnector : private CallersContainer
	{
	protected:
		using SlotInfos = List<_Slots, Infos...>;

		template<_Slots slot>
		using FindSlotInfo =
			Utils::FindSlotInfo<_Slots, slot, SlotInfos, single>::SlotInfo;

	public:
		using Slots = IsUniqueList<_Slots, Infos...>::Slots; // check Infos is unique

		template<Slots slot>
		using SignalPosition = CallersContainer::template SignalPosition<slot>;

		virtual ~BaseSignalsConnector() = default;

		template<_Slots slot, auto function>
		SignalPosition<slot> signal_connect()
		{
			using S = FindSlotInfo<slot>;

			using T = FunctionTraits<function>;
			using ST = S::Function;

			check<T, ST>();

			using C = S::Callers::template Function<function>;
			return CallersContainer::template signal_add<slot>(new C);
		}

		template<_Slots slot, auto method,
				 typename Traits = MethodTraits<method>,
				 typename OwnerType = Traits::Owner,
				 typename Owner = std::conditional_t<Traits::is_const,
													 const OwnerType,
													 OwnerType>>
		SignalPosition<slot>
			signal_connect(std::remove_reference_t<PtrContainer<Owner>> owner)
		{
			using S = FindSlotInfo<slot>;
			using ST = S::template Method<Owner>;

			check<Traits, ST>();

			using C = S::Callers::template Method<Owner, method>;
			return CallersContainer::template signal_add<slot>(new C(owner));
		}

		template<_Slots slot, typename Functor>
		SignalPosition<slot> signal_connect(Functor&& functor)
		{
			using S = FindSlotInfo<slot>;

			using F = ClearType<Functor>;

			using T = LambdaTraits<F>;
			using ST = S::template Method<F>;

			check<T, ST>();

			using C = S::Callers::template Functor<F>;
			return CallersContainer::template
				signal_add<slot>(new C(std::forward<Functor>(functor)));
		}

		using CallersContainer::signal_disconnect;

	protected:
		using CallersContainer::signal_disconnect_all;

		using CallersContainer::signal_get;

	private:
		template<typename T>
		struct ScanTypes {};

		template<typename... AArgs, typename... BArgs>
		static consteval bool check_params(ScanTypes<std::tuple<AArgs...>>,
										   ScanTypes<std::tuple<BArgs...>>)
			{ return (std::is_convertible_v<AArgs, BArgs> && ...); }

		template<typename Traits, typename SlotTraits>
		static consteval void check() noexcept
		{
			using Args = Traits::Args;
			using SArgs = SlotTraits::Args;

			static constexpr bool is_noexpect = Traits::is_noexcept;
			static constexpr bool s_is_noexcept = SlotTraits::is_noexcept;

			static_assert(std::is_void_v<typename Traits::Result>,
						  "Result type not void");
			static_assert(s_is_noexcept == is_noexpect || !s_is_noexcept,
						  "Required noexcept");
			static_assert(check_params(ScanTypes<SArgs>{}, ScanTypes<Args>{}),
						  "Arguments don't convertible");
		}
	};
}
