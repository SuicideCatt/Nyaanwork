module;

#include <Nyaanwork/WindowSystem.hpp>

#include <concepts>
#include <exception>
#include <iostream>
#include <print>

export module Nyaanwork.WindowSystem:loader;
import :Exception;
import :WindowSystem;

#define BACKENDS_NAMESPACE Nyaanwork::WindowSystem::Backends
#define GET_BACKEND(name) BACKENDS_NAMESPACE::name::Instance
#define GET_NO_BACKEND(name) void

#ifdef NYAANWORK_WINDOW_SYSTEM_BACKENDS_LINUX
	#ifdef NYAANWORK_WINDOW_SYSTEM_BACKENDS_LINUX_XCB
		import Nyaanwork.WindowSystem.Backends.Linux.XCB;
		#define LINUX_XCB GET_BACKEND(Linux::XCB)
	#else
		#define LINUX_XCB GET_NO_BACKEND(void)
	#endif

	#define LINUX LINUX_XCB, void
#else
	#define LINUX void
#endif

#ifdef NYAANWORK_WINDOW_SYSTEM_BACKENDS_WINDOWS
	import Nyaanwork.WindowSystem.Backends.Windows;
	#define WINDOWS GET_BACKEND(Windows)
#else
	#define WINDOWS GET_NO_BACKEND(Windows)
#endif

#define MAIN_BACKENDS LINUX, WINDOWS, void

namespace Nyaanwork::WindowSystem
{
	// remove void types from backends list
	template<typename F, typename... Backends, typename... OnList>
	consteval auto push_list(Instance::Custom<OnList...> backends)
	{
		if constexpr (sizeof...(Backends) == 0)
		{
			if constexpr (std::derived_from<F, Instance::Base>)
				return Instance::Custom<OnList..., F>();
			else
				return Instance::Custom<OnList...>();
		}
		else
		{
			if constexpr (std::derived_from<F, Instance::Base>)
				return push_list<Backends...>(Instance::Custom<OnList..., F>());
			else
				return push_list<Backends...>(Instance::Custom<OnList...>());
		}
	}

	template<typename... Backends>
	consteval auto make_list()
		{ return push_list<Backends...>(Instance::Custom<>()); }

	static constexpr auto main_backends = make_list<MAIN_BACKENDS>();
}

export namespace Nyaanwork::WindowSystem
{
	template<std::derived_from<Instance::Base>... Backends>
	Instance::Instance(Opt<Str> window_class_name, Custom<Backends...> backends)
		: Instance(std::move(window_class_name), main_backends, backends) {}

	template<std::derived_from<Instance::Base>... MainBackends,
			 std::derived_from<Instance::Base>... Backends>
	requires (sizeof...(MainBackends) > 0 || sizeof...(Backends) > 0)
	Instance::Instance(Opt<Str> window_class_name,
					   Custom<MainBackends...>, Custom<Backends...>)
	{
		static constexpr auto mains = sizeof...(MainBackends);
		static constexpr auto customs = sizeof...(Backends);

		static constexpr auto backends = customs+mains;

		// if mains+customs backends size is 1 can throw exception
		// if size > 1 just print exception message and if any backends not
		//   inited throw error

		bool inited = false;
		inited = load_backends<backends != 1,
							   Backends..., MainBackends...>(window_class_name);

		if (!inited)
			throw Error::Exception(Error::Code::fail_to_init_any_instance);
	}

	template<bool no_throw, std::derived_from<Instance::Base>... Backends>
	bool Instance::load_backends(const Opt<Str>& window_class_name) noexcept(no_throw)
	{
		return (([this, &window_class_name] noexcept(no_throw) -> bool
		{
			if constexpr (no_throw)
			{
				try
				{
					m_base = make_shared<Backends>(window_class_name);
					return true;
				}
				catch (const std::exception& e)
				{
					std::println(std::cerr, "{}", e.what());
					return false;
				}
			}
			else
			{
				m_base = make_shared<Backends>(window_class_name);
				return true;
			}
		}()) || ...);
	}
}
