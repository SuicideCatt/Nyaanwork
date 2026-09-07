module;

#include <concepts>

export module Nyaanwork.WindowSystem:WindowSystem;

export import Nyaanwork.WindowSystem.Backends.Base;
export import Nyaanwork.WindowSystem.Codes;
import Nyaanwork.Core.Types;

export namespace Nyaanwork::WindowSystem
{
	using InputState = Backends::Base::InputState;
	using Window = Backends::Base::Window;

	class Instance final
	{
	public:
		using Base = Backends::Base::Instance;

		using Slots = Base::Slots;
		template<Slots slot>
		using SignalPosition = Base::SignalPosition<slot>;

		using Displays = Base::Displays;

		template<std::derived_from<Base>... Backends>
		struct Custom {};

		template<std::derived_from<Base>... Backends>
		Instance(Opt<Str> window_class_name = nullopt,
				 Custom<Backends...> backends = {});

		Instance(const Instance&) = default;
		Instance& operator=(const Instance&) = default;

		Instance(Instance&&) = default;
		Instance& operator=(Instance&&) = default;

		Base& operator*()
			{ return *m_base; }

		const Base& operator*() const
			{ return *m_base; }

		Base* operator->()
			{ return m_base.get(); }

		const Base* operator->() const
			{ return m_base.get(); }

		Ptr<Base> base() const
			{ return m_base; }

	private:
		template<std::derived_from<Base>... MainBackends,
				 std::derived_from<Base>... Backends>
		requires (sizeof...(MainBackends) > 0 || sizeof...(Backends) > 0)
		Instance(Opt<Str> window_class_name,
				 Custom<MainBackends...>, Custom<Backends...>);

		template<bool no_throw, std::derived_from<Base>... Backends>
		bool load_backends(const Opt<Str>& window_class_name) noexcept(no_throw);

		Ptr<Base> m_base;
	};
}
