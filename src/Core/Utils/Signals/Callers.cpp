module;

#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

export module Nyaanwork.Core.Utils.Signals:Callers;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.PtrContainer;

namespace Nyaanwork::Utils
{
	template<typename _Type>
	struct Solved
	{
		using Type = _Type;
	};

	// all Solve hack neaded for BaseCaller::call method
	// converts all "Type" to "const Type&"
	// i don't want make copy be often
	// maybe find better way...
	template<typename _Type>
	struct SolveT : Solved<const _Type&> {};

	template<typename _Type>
	struct SolveT<_Type&> : Solved<_Type&> {};

	template<typename _Type>
	struct SolveT<_Type&&> : Solved<_Type&&> {};

	template<typename _Type>
	struct SolveT<const _Type&> : Solved<const _Type&> {};

	template<typename Type>
	using Solve = SolveT<Type>::Type;

	struct BaseContainer
	{
		virtual ~BaseContainer() = default;
	};

	template<bool is_noexcept, typename... Args>
	struct BaseCaller : BaseContainer
	{
		virtual ~BaseCaller() = default;

		virtual void call(Solve<Args>... args) noexcept(is_noexcept) = 0;
	};

	template<bool is_noexcept, auto function, typename... Args>
	struct FunctionCaller final : BaseCaller<is_noexcept, Args...>
	{
		void call(Solve<Args>... args) noexcept(is_noexcept) override
			{ function(args...); }
	};

	template<bool is_noexcept, typename Owner, auto method, typename... Args>
	struct MethodCaller final : BaseCaller<is_noexcept, Args...>
	{
		MethodCaller(PtrContainer<Owner> c) noexcept
			: c(c) {}

		void call(Solve<Args>... args) noexcept(is_noexcept) override
			{ (c.ptr()->*method)(args...); }

		PtrContainer<Owner> c;
	};

	template<bool is_noexcept, typename Functor, typename... Args>
	struct FunctorCaller final : BaseCaller<is_noexcept, Args...>
	{
		template<typename _Functor>
		FunctorCaller(_Functor&& f) noexcept
			: f(f) {}

		void call(Solve<Args>... args) noexcept(is_noexcept) override
			{ f(args...); }

		Functor f;
	};

	// single thread CallersContainer
	template<is_scoped_enum Slots>
	class CallersContainer
	{
	public:
		template<Slots slot_>
		struct SignalPosition
		{
			static constexpr Slots slot = slot_;
			usize id = 0;
		};

		virtual ~CallersContainer() = default;

		template<Slots slot>
		void signal_disconnect(SignalPosition<slot> pos)
		{
			auto& [last_id, callers] = m_signals[slot];
			std::erase_if(callers, [&pos](const auto& caller)
				{ return pos.id == caller.first; });

			if (callers.empty())
				last_id = 0;
		}

	protected:
		template<Slots slot>
		SignalPosition<slot> signal_add(BaseContainer* container)
		{
			auto& [last_id, callers] = m_signals[slot];
			auto& [id, caller] = callers.emplace_back(last_id++, container);
			return {id};
		}

	private:
		using Caller = Pair<usize, UPtr<BaseContainer>>;
		using Callers = std::vector<Caller>;

	protected:
		Pair<i32, Callers&> signal_get(Slots slot)
			{ return {0, m_signals[slot].second};}

		template<Slots slot>
		void signal_disconnect_all()
			{ signal_disconnect_all(slot); }

		void signal_disconnect_all(Slots slot)
		{
			auto& [last_id, callers] = m_signals[slot];
			callers.clear();
			last_id = 0;
		}

		void signal_disconnect_all()
			{ m_signals.clear(); }

	private:
		std::unordered_map<Slots, Pair<usize, Callers>> m_signals;
	};

	// multithread version of CallersContainer
	template<is_scoped_enum Slots>
	class SyncedCallersContainer : private CallersContainer<Slots>
	{
	private:
		using CallersContainer = typename SyncedCallersContainer::CallersContainer;

	public:
		template<Slots slot>
		using SignalPosition = CallersContainer::template SignalPosition<slot>;

		virtual ~SyncedCallersContainer() = default;

		template<Slots slot>
		void signal_disconnect(SignalPosition<slot> pos)
		{
			std::unique_lock l(m_mutex);
			CallersContainer::template signal_disconnect<slot>(pos);
		}

	protected:
		template<Slots slot>
		SignalPosition<slot> signal_add(BaseContainer* container)
		{
			std::unique_lock l(m_mutex);
			return CallersContainer::template signal_add<slot>(container);
		}

	private:
		using Caller = Pair<usize, UPtr<BaseContainer>>;
		using Callers = std::vector<Caller>;

	protected:
		Pair<std::shared_lock<std::shared_mutex>, Callers&>
			signal_get(Slots slot)
		{
			return {
				std::shared_lock(m_mutex),
				CallersContainer::signal_get(slot).second
			};
		}

		template<Slots slot>
		void signal_disconnect_all()
			{ signal_disconnect_all(slot); }

		void signal_disconnect_all(Slots slot)
		{
			std::unique_lock l(m_mutex);
			CallersContainer::signal_disconnect_all(slot);
		}

		void signal_disconnect_all()
		{
			std::unique_lock l(m_mutex);
			CallersContainer::signal_disconnect_all();
		}

	private:
		std::shared_mutex m_mutex;
	};
}
