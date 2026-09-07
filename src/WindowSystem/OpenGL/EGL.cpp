module;

#include <Core/Defines.hpp>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <span>
#include <vector>

export module Nyaanwork.WindowSystem.OpenGL:EGL;
import :API;
import :Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.ClearType;
import Nyaanwork.Core.Utils.UPtr;

namespace Nyaanwork::WindowSystem::OpenGL::EGL
{
	struct Field
	{
		EGLint name;
		EGLint value;
	};

	auto make_fields(usize fields_count)
	{
		std::vector<EGLint> config((sizeof(Field)/4)*fields_count + 1);
		config.back() = EGL_NONE;
		std::span fields(reinterpret_cast<Field*>(config.data()),
						 config.size()/(sizeof(Field)/4));

		return Pair{std::move(config), fields};
	}

	class Instance
	{
	public:
		using API = OpenGL::API;

		template<typename N>
		Instance(API api, N native_display)
			: m_api(api), m_swap_interval(0)
		{
			m_display.reset(eglGetDisplay(native_display));
			auto d = m_display.get();
			if (d == EGL_NO_DISPLAY) NYAAN_UNLIK
				throw Error::Exception(Error::Code::cannot_get_egl_display);

			if (!eglInitialize(d, nullptr, nullptr)) NYAAN_UNLIK
				throw Error::Exception(Error::Code::fail_to_init_elg_display);

			EGLint render;
			switch (api)
			{
			case API::gl:
				render = EGL_OPENGL_BIT;
				break;
			}

			auto [config, fields] = make_fields(6);
			fields[0] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT};
			fields[1] = {EGL_RENDERABLE_TYPE, render};
			fields[2] = {EGL_RED_SIZE, 8};
			fields[3] = {EGL_GREEN_SIZE, 8};
			fields[4] = {EGL_BLUE_SIZE, 8};
			fields[5] = {EGL_DEPTH_SIZE, 24};

			EGLint num_config;
			eglChooseConfig(display(), config.data(), &m_config, 1, &num_config);
		}

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;

		Instance(Instance&&) = delete;
		Instance& operator=(Instance&&) = delete;

		API api() NYAAN_CNOEX
			{ return m_api; }

		void bind_api()
		{
			auto get_api = [this]()
			{
				switch (api())
				{
				case API::gl:
					return EGL_OPENGL_API;
				}
			};

			if (!eglBindAPI(get_api()) || eglGetError() != EGL_SUCCESS) NYAAN_UNLIK
				throw Error::Exception(Error::Code::cannot_bind_api);
		}

		void wait()
			{ eglWaitClient(); }

		EGLDisplay display() NYAAN_CNOEX
			{ return m_display.get(); }

		EGLConfig config() NYAAN_CNOEX
			{ return m_config; }

		u8 swap_interval() NYAAN_CNOEX
			{ return m_swap_interval; }

		void swap_interval(u8 interval) noexcept
			{ eglSwapInterval(display(), (m_swap_interval = interval)); }

	private:
		GetUPtr<ClearType<EGLDisplay>, eglTerminate> m_display;
		EGLConfig m_config;
		API m_api;
		u8 m_swap_interval;
	};

	class Surface
	{
	public:
		EGLSurface surface() NYAAN_CNOEX
			{ return m_surface; }

	protected:
		template<typename N>
		void init(Instance& instance, N native_window)
		{
			if (m_surface != EGL_NO_SURFACE) NYAAN_UNLIK
				throw Error::Exception("Surface", Error::Code::already_created);

			auto d = instance.display();
			auto c = instance.config();

			m_surface = eglCreateWindowSurface(d, c, native_window, nullptr);

			if (m_surface == EGL_NO_SURFACE) NYAAN_UNLIK
				throw Error::Exception("Surface", Error::Code::fail_to_create);
		}

		void swap_buffers(Instance& instance) noexcept
			{ eglSwapBuffers(instance.display(), m_surface); }

		template<bool check = true>
		void destroy(Instance& instance)
		{
			if (m_surface == EGL_NO_SURFACE) NYAAN_UNLIK
				return;

			auto d = instance.display();

			if constexpr (check)
				if (eglGetCurrentSurface(EGL_DRAW) == m_surface)
					eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

			eglDestroySurface(d, m_surface);
			m_surface = nullptr;
		}

	private:
		EGLSurface m_surface = EGL_NO_SURFACE;
	};

	class Context
	{
	public:
		using Profile = OpenGL::Profile;

		EGLContext context() NYAAN_CNOEX
			{ return m_context; }

		bool is_current() NYAAN_CNOEX
			{ return eglGetCurrentContext() == m_context; }

		u8 swap_interval() NYAAN_CNOEX
			{ return m_swap_interval; }

		using ProcType = __eglMustCastToProperFunctionPointerType;
		static ProcType get_proc_address(const char* proc_name)
			{ return eglGetProcAddress(proc_name); }

	protected:
		void init(Instance& instance, Profile profile, vec2<u8> min_version)
		{
			if (m_context != EGL_NO_CONTEXT) NYAAN_UNLIK
				throw Error::Exception("Context", Error::Code::already_created);

			std::vector<EGLint> config;

			switch (instance.api())
			{
			case Instance::API::gl:
				{
					static constexpr u8 versions[] = {1, 3, 6};

					if (min_version.x < 2 || min_version.x > 4) NYAAN_UNLIK
						throw Error::Exception("Major version", Error::Code::wrong_version);

					if (min_version.y > versions[min_version.x-2]) NYAAN_UNLIK
						throw Error::Exception("Minor version", Error::Code::wrong_version);

					auto [nconfig, fields] = make_fields(3);
					fields[0] = {EGL_CONTEXT_MAJOR_VERSION, min_version.x};
					fields[1] = {EGL_CONTEXT_MINOR_VERSION, min_version.y};
					fields[2].name = EGL_CONTEXT_OPENGL_PROFILE_MASK;
					if (profile == Profile::core)
						fields[2].value = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
					else
						fields[2].value = EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT;
					config = std::move(nconfig);
				}
				break;
			}

			auto d = instance.display();
			auto c = instance.config();

			m_context = eglCreateContext(d, c, EGL_NO_CONTEXT, config.data());

			if (m_context == EGL_NO_CONTEXT) NYAAN_UNLIK
				throw Error::Exception("Context", Error::Code::fail_to_create);
		}

		void make_current(Instance& instance, Surface& surface)
		{
			if (is_current()) NYAAN_UNLIK
				return;

			instance.bind_api();
			auto surf = surface.surface();
			eglMakeCurrent(instance.display(), surf, surf, m_context);
			instance.swap_interval(m_swap_interval);
		}

		void swap_interval_impl(Instance& instance, u8 interval)
		{
			if (m_swap_interval == interval) NYAAN_UNLIK
				return;

			m_swap_interval = interval;

			if (!is_current()) NYAAN_UNLIK
				return;

			instance.swap_interval(interval);
		}

		template<bool check = true>
		void destroy(Instance& instance)
		{
			if (m_context == EGL_NO_CONTEXT) NYAAN_UNLIK
				return;

			auto d = instance.display();

			if constexpr (check)
				if (is_current())
					eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

			eglDestroyContext(d, m_context);
			m_context = nullptr;
		}

	private:
		EGLContext m_context = EGL_NO_CONTEXT;
		u8 m_swap_interval = 0;
	};
}
