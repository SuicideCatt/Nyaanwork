module;

#include <Core/Defines.hpp>

#include <exception>

export module Nyaanwork.WindowSystem.OpenGL;
export import :Exception;
import :API;

import Nyaanwork.Core.Types;

#ifdef _WIN32
	import :WGL;
	#define PLATFORM WGL
#else
	import :EGL;
	#define PLATFORM EGL
#endif

namespace Nyaanwork::WindowSystem::OpenGL::Platform
{
	using Instance = PLATFORM::Instance;
	using Surface = PLATFORM::Surface;
	using Context = PLATFORM::Context;
}

export namespace Nyaanwork::WindowSystem::OpenGL
{
	class Context;

	class Instance final
		: public EnableSharedFromThis<Instance>, private Platform::Instance
	{
	public:
		using API = OpenGL::API;

		template<typename N>
		Instance(API api, N native_display)
			: Platform::Instance(api, native_display) {}

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;

		Instance(Instance&&) = delete;
		Instance& operator=(Instance&&) = delete;

		using Platform::Instance::api;
		using Platform::Instance::bind_api;
		using Platform::Instance::wait;

		Ptr<Context> create_context(Profile profile, vec2<u8> min_version)
			{ return make_shared<Context>(shared_from_this(), profile, min_version); }

		Platform::Instance& base() noexcept
			{ return static_cast<Platform::Instance&>(*this); }
		const Platform::Instance& base() NYAAN_CNOEX
			{ return static_cast<const Platform::Instance&>(*this); }
	};

	class Surface final : private Platform::Surface
	{
	public:
		template<typename N>
		Surface(Ptr<Instance> instance, N native_window)
			: m_instance(instance)
		{
			auto& i = *instance;
			i.bind_api();
			Platform::Surface::init(i.base(), native_window);
		}

		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;

		Surface(Surface&&) = delete;
		Surface& operator=(Surface&&) = delete;

		~Surface()
		{
			auto& i = *m_instance;

			i.wait();

			try
			{
				i.bind_api();
			}
			catch (const std::exception&) {};

			Platform::Surface::destroy<true>(i.base());
		}

		Ptr<Instance> instance() const
			{ return m_instance; }

		void swap_buffers()
			{ Platform::Surface::swap_buffers(m_instance->base()); }

		Platform::Surface& base() noexcept
			{ return static_cast<Platform::Surface&>(*this); }
		const Platform::Surface& base() NYAAN_CNOEX
			{ return static_cast<const Platform::Surface&>(*this); }

	private:
		Ptr<Instance> m_instance;
	};

	class Context final : private Platform::Context
	{
	public:
		using Profile = OpenGL::Profile;

		Context(Ptr<Instance> instance, Profile profile, vec2<u8> min_version)
			: m_instance(instance)
		{
			auto& i = *instance;

			i.bind_api();
			Platform::Context::init(i.base(), profile, min_version);
		}

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		Context(Context&&) = delete;
		Context& operator=(Context&&) = delete;

		~Context()
		{
			auto& i = *m_instance;

			i.wait();

			try
			{
				i.bind_api();
			}
			catch (const std::exception&) {};

			Platform::Context::destroy<true>(i.base());
		}

		Ptr<Instance> instance() const
			{ return m_instance; }

		using Platform::Context::is_current;
		void make_current(Ptr<Surface> surface)
			{ Platform::Context::make_current(m_instance->base(), surface->base()); }

		using Platform::Context::swap_interval;
		void swap_interval(u8 interval)
			{ Platform::Context::swap_interval_impl(m_instance->base(), interval); }

		using Platform::Context::ProcType;
		using Platform::Context::get_proc_address;

		Platform::Context& base() noexcept
			{ return static_cast<Platform::Context&>(*this); }
		const Platform::Context& base() NYAAN_CNOEX
			{ return static_cast<const Platform::Context&>(*this); }

	private:
		Ptr<Instance> m_instance;
	};

	class SurfaceContext final
		: private Platform::Surface, private Platform::Context
	{
	public:
		using Profile = OpenGL::Profile;

		template<typename N>
		SurfaceContext(Ptr<Instance> instance,
					   Profile profile, vec2<u8> min_version,
					   N native_window)
			: m_instance(instance)
		{
			auto& i = *instance;

			i.bind_api();

			auto& b = i.base();
			Platform::Surface::init(b, native_window);
			Platform::Context::init(b, profile, min_version);
		}

		SurfaceContext(const SurfaceContext&) = delete;
		SurfaceContext& operator=(const SurfaceContext&) = delete;

		SurfaceContext(SurfaceContext&&) = delete;
		SurfaceContext& operator=(SurfaceContext&&) = delete;

		~SurfaceContext()
		{
			auto& i = *m_instance;

			i.wait();

			try
			{
				i.bind_api();
			}
			catch (const std::exception&) {};

			auto& b = i.base();
			Platform::Context::destroy<true>(b);
			Platform::Surface::destroy<false>(b);
		}

		Ptr<Instance> instance() const
			{ return m_instance; }

		using Platform::Context::is_current;
		void make_current()
			{ Platform::Context::make_current(m_instance->base(), base_surface()); }

		using Platform::Context::swap_interval;
		void swap_interval(u8 interval)
			{ swap_interval_impl(m_instance->base(), interval); }

		void swap_buffers()
			{ Platform::Surface::swap_buffers(m_instance->base()); }

		using Platform::Context::ProcType;
		using Platform::Context::get_proc_address;

		Platform::Surface& base_surface() noexcept
			{ return static_cast<Platform::Surface&>(*this); }
		const Platform::Surface& base_surface() NYAAN_CNOEX
			{ return static_cast<const Platform::Surface&>(*this); }

		Platform::Context& base_context() noexcept
			{ return static_cast<Platform::Context&>(*this); }
		const Platform::Context& base_context() NYAAN_CNOEX
			{ return static_cast<const Platform::Context&>(*this); }

	private:
		Ptr<Instance> m_instance;
	};
}
