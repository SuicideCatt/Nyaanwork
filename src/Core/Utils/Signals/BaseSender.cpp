module;

#include <concepts>
#include <utility>

export module Nyaanwork.Core.Utils.Signals:BaseSender;
import :Connector;

import Nyaanwork.Core.Utils.Flags;

namespace Nyaanwork::Utils
{
	template<typename T>
	struct ScanTypes {};

	template<typename... AArgs, typename... BArgs>
	consteval bool check_params(ScanTypes<std::tuple<BArgs...>>)
		{ return (std::is_convertible_v<AArgs, BArgs> && ...); }

	// BaseSignalsSender for SlotInfo
	template<is_scoped_enum _Slots, typename CallersContainer, typename... Infos>
	class BaseSignalsSender
		: private BaseSignalsConnector<_Slots, false, CallersContainer, Infos...>
	{
	private:
		using Connector = BaseSignalsSender::BaseSignalsConnector;

	public:
		using typename Connector::Slots;

		template<_Slots slot>
		using SignalPosition = Connector::template SignalPosition<slot>;

		virtual ~BaseSignalsSender() = default;

		using Connector::signal_connect;
		using Connector::signal_disconnect;

	protected:
		template<_Slots slot, typename... Args>
		void signal_send(Args&&... args)
		{
			using S = Connector::template FindSlotInfo<slot>;
			using C = S::Callers::Base;

			using Scan = ScanTypes<typename S::Function::Args>;
			static_assert(check_params<Args...>(Scan()),
						  "Call arguments don't convertible");

			auto [l, callers] = Connector::signal_get(slot);
			for (auto& [id, caller] : callers)
				static_cast<C*>(caller.get())->call(args...);
		}

		using Connector::signal_disconnect_all;
	};

	// BaseSignalsSender for SingleSlotInfo
	template<is_scoped_enum _Slots, typename CallersContainer, typename SingleInfo>
	requires SingleInfo::single
	class BaseSignalsSender<_Slots, CallersContainer, SingleInfo>
		: private BaseSignalsConnector<_Slots, true, CallersContainer,
							typename SingleInfo::template MultiSlotInfo<_Slots>>
	{
	private:
		using Connector = BaseSignalsSender::BaseSignalsConnector;

	public:
		using typename Connector::Slots;

		template<_Slots slot>
		using SignalPosition = Connector::template SignalPosition<slot>;

		virtual ~BaseSignalsSender() = default;

		using Connector::signal_connect;
		using Connector::signal_disconnect;

	protected:
		template<_Slots slot, typename... Args>
		void signal_send(Args&&... args)
			{ signal_send(slot, std::forward<Args>(args)...); }

		template<typename... Args>
		void signal_send(_Slots slot, Args&&... args)
		{
			using S = Connector::template FindSlotInfo<Slots{}>;
			using C = S::Callers::Base;

			using Scan = ScanTypes<typename S::Function::Args>;
			static_assert(check_params<Args...>(Scan()),
						  "Call arguments don't convertible");

			auto [l, callers] = Connector::signal_get(slot);
			for (auto& [id, caller] : callers)
				static_cast<C*>(caller.get())->call(args...);
		}

		using Connector::signal_disconnect_all;
	};
}
