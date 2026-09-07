// SignalsSender and SyncedSignalsSender is layered pie
// 1. CallersContainer or SyncedCallersContainer
//    CallersContainer is single thread version
//    SyncedCallersContainer is multithread version of CallersContainer
// 2. BaseSignalsConnector - universal
// 3. BaseSignalsSender or BaseSignalsSender for SingleSlotInfo

export module Nyaanwork.Core.Utils.Signals;
export import :SlotInfo;
import :BaseSender;
import :Callers;

import Nyaanwork.Core.Utils.Flags;

export namespace Nyaanwork::Utils
{
	template<is_scoped_enum _Slots, typename... Infos>
	class SignalsSender
		: private BaseSignalsSender<_Slots, CallersContainer<_Slots>, Infos...>
	{
	private:
		using BaseSender = SignalsSender::BaseSignalsSender;

	public:
		using typename BaseSender::Slots;

		template<_Slots slot>
		using SignalPosition = BaseSender::template SignalPosition<slot>;

		virtual ~SignalsSender() = default;

		using BaseSender::signal_connect;
		using BaseSender::signal_disconnect;

	protected:
		SignalsSender() = default;

		using BaseSender::signal_send;
		using BaseSender::signal_disconnect_all;
	};

	template<is_scoped_enum _Slots, typename... Infos>
	class SyncedSignalsSender
		: private BaseSignalsSender<_Slots, SyncedCallersContainer<_Slots>, Infos...>
	{
	private:
		using BaseSender = SyncedSignalsSender::BaseSignalsSender;

	public:
		using typename BaseSender::Slots;

		template<_Slots slot>
		using SignalPosition = BaseSender::template SignalPosition<slot>;

		virtual ~SyncedSignalsSender() = default;

		using BaseSender::signal_connect;
		using BaseSender::signal_disconnect;

	protected:
		SyncedSignalsSender() = default;

		using BaseSender::signal_send;
		using BaseSender::signal_disconnect_all;
	};
}

export namespace Nyaanwork
{
	using namespace Utils;
}
