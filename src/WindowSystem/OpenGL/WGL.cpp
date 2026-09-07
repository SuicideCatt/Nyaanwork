module;

#include <Core/Defines.hpp>

#include <windows.h>
#include <psapi.h>

#include <filesystem>
#include <string>

export module Nyaanwork.WindowSystem.OpenGL:WGL;
import :API;
import :Exception;

import Nyaanwork.Core.Types;

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092

#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

namespace Nyaanwork::WindowSystem::OpenGL::WGL::Func
{
	using SwapBuffers = BOOL(*)(HDC);
	using CreateContextAttribsARB = HGLRC(*)(HDC, HGLRC share_context, const int* attribs);
	using SwapInterval = BOOL(*)(int);
}

export namespace Nyaanwork::WindowSystem::OpenGL::WGL
{
	class Instance
	{
	public:
		using API = OpenGL::API;

		struct Module
		{
			Func::SwapBuffers swap_buffers = nullptr;
			Func::CreateContextAttribsARB create_context = nullptr;
			Func::SwapInterval swap_interval = nullptr;
		};

		template<typename N>
		Instance(API api, N native_display)
			: m_api(api)
		{
			DWORD size = 1;
			EnumProcessModules(GetCurrentProcess(), nullptr, 0, &size);
			std::vector<HMODULE> modules(size);
			EnumProcessModules(GetCurrentProcess(),
							   modules.data(), sizeof(HMODULE) * size, &size);

			HMODULE gl = nullptr;
			std::wstring result(MAX_PATH, '\0');
			for (auto m : modules)
			{
				auto len = GetModuleFileNameW(m, result.data(), result.size());
				std::wstring name(result.data(), len);
				std::transform(name.begin(), name.end(),
							   name.begin(), [](char c) { return std::toupper(c); });
				std::filesystem::path path = name;
				name = path.filename().wstring();
				if (name == L"OPENGL32.DLL") NYAAN_UNLIK
				{
					gl = m;
					break;
				}
			}
			if (!gl)
				throw Error::Exception(Error::Code::opengl32_not_loaded);

			m_module.swap_buffers =
				reinterpret_cast<Func::SwapBuffers>(GetProcAddress(gl, "wglSwapBuffers"));
			if (!m_module.swap_buffers) NYAAN_UNLIK
				throw Error::Exception("wglSwapBuffers", Error::Code::no_wgl_function);
		}

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;

		Instance(Instance&&) = delete;
		Instance& operator=(Instance&&) = delete;

		API api() NYAAN_CNOEX
			{ return m_api; }

		void bind_api() {}

		void wait() {}

		bool module_ready() NYAAN_CNOEX
		{
			return m_module.swap_buffers
				&& m_module.create_context
				&& m_module.swap_interval;
		}

		void init_module(HDC native_surface)
		{
			HGLRC legacy = wglCreateContext(native_surface);
			if (!legacy) NYAAN_UNLIK
			{
				throw Error::Exception("Legacy OpenGL context",
									   Error::Code::fail_to_create);
			}
			wglMakeCurrent(native_surface, legacy);

			auto func = wglGetProcAddress("wglSwapIntervalEXT");
			m_module.swap_interval = reinterpret_cast<Func::SwapInterval>(func);
			if (!m_module.swap_interval) NYAAN_UNLIK
				throw Error::Exception("wglSwapIntervalEXT", Error::Code::no_wgl_function);

			func = wglGetProcAddress("wglCreateContextAttribsARB");
			m_module.create_context = reinterpret_cast<Func::CreateContextAttribsARB>(func);
			if (!m_module.create_context) NYAAN_UNLIK
			{
				throw Error::Exception("wglCreateContextAttribsARB",
									   Error::Code::no_wgl_function);
			}

			wglDeleteContext(legacy);
		}

		Module module() NYAAN_CNOEX
			{ return m_module; }

	private:
		Module m_module;
		API m_api;
	};

	class Surface
	{
	public:
		HDC surface() NYAAN_CNOEX
			{ return m_surface; }

	protected:
		void init(Instance& instance, HWND native_window)
		{
			if (m_surface) NYAAN_UNLIK
				throw Error::Exception("Surface", Error::Code::already_created);

			PIXELFORMATDESCRIPTOR pfd =
			{
				.nSize = sizeof(PIXELFORMATDESCRIPTOR),
				.nVersion = 1,
				.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				.iPixelType = PFD_TYPE_RGBA,
				.cColorBits = 32,
				.cRedBits = 0, .cRedShift = 0,
				.cGreenBits = 0, .cGreenShift = 0,
				.cBlueBits = 0, .cBlueShift = 0,
				.cAlphaBits = 0, .cAlphaShift = 0,
				.cAccumBits = 0,
				.cAccumRedBits = 0, .cAccumGreenBits = 0,
				.cAccumBlueBits = 0, .cAccumAlphaBits = 0,
				.cDepthBits = 24,
				.cStencilBits = 8,
				.cAuxBuffers = 0,
				.iLayerType = PFD_MAIN_PLANE,
				.bReserved = 0,
				.dwLayerMask = 0, .dwVisibleMask = 0, .dwDamageMask = 0
			};

			m_surface = GetDC(native_window);
			if (!m_surface) NYAAN_UNLIK
			{
				throw Error::Exception("Surface: fail to get DC",
									   Error::Code::fail_to_create);
			}

			int pixel_format = ChoosePixelFormat(m_surface, &pfd);
			if (pixel_format == 0) NYAAN_UNLIK
			{
				throw Error::Exception("Surface: no pixel format",
									   Error::Code::fail_to_create);
			}

			if (!SetPixelFormat(m_surface, pixel_format, &pfd)) NYAAN_UNLIK
			{
				throw Error::Exception("Surface: can't set pixel format",
									   Error::Code::fail_to_create);
			}
		}

		void swap_buffers(Instance& instance) noexcept
			{ instance.module().swap_buffers(m_surface); }

		template<bool check = true>
		void destroy(Instance& instance)
			{ m_surface = nullptr; }

	private:
		HDC m_surface = nullptr;
	};

	class Context
	{
	public:
		using Profile = OpenGL::Profile;

		HGLRC context() NYAAN_CNOEX
			{ return m_context; }

		bool is_current() NYAAN_CNOEX
			{ return wglGetCurrentContext() == m_context; }

		u8 swap_interval() NYAAN_CNOEX
			{ return m_swap_interval; }

		using ProcType = PROC;
		static ProcType get_proc_address(const char* proc_name)
			{ return wglGetProcAddress(proc_name); }

	protected:
		void init(Instance& instance, Profile profile, vec2<u8> min_version)
		{
			m_profile = profile;
			m_min_version = min_version;
		}

		void init(Instance& instance, HDC native_surface,
				  Profile profile, vec2<u8> min_version)
		{
			if (m_context) NYAAN_UNLIK
				throw Error::Exception("Context", Error::Code::already_created);

			if (!instance.module_ready()) NYAAN_UNLIK
				return;

			int attribs[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_PROFILE_MASK_ARB,
					instance.api() == API::gl?
						WGL_CONTEXT_CORE_PROFILE_BIT_ARB
							: WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
				0
			};

			m_context =
				instance.module().create_context(native_surface, nullptr, attribs);
			if (!m_context) NYAAN_UNLIK
				throw Error::Exception("Context", Error::Code::fail_to_create);

			m_swap_interval = 0;
		}

		void make_current(Instance& instance, Surface& surface)
		{
			if (!instance.module_ready()) NYAAN_UNLIK
				instance.init_module(surface.surface());

			if (!m_context) NYAAN_UNLIK
				init(instance, surface.surface(), m_profile, m_min_version);

			if (is_current()) NYAAN_UNLIK
				return;

			instance.bind_api();
			wglMakeCurrent(surface.surface(), m_context);
			instance.module().swap_interval(m_swap_interval);
		}

		void swap_interval_impl(Instance& instance, u8 interval)
		{
			if (m_swap_interval == interval) NYAAN_UNLIK
				return;

			m_swap_interval = interval;

			if (!instance.module_ready()) NYAAN_UNLIK
				return;

			if (!is_current()) NYAAN_UNLIK
				return;

			instance.module().swap_interval(interval);
		}

		template<bool check = true>
		void destroy(Instance& instance)
		{
			if (!instance.module_ready() || !m_context) NYAAN_UNLIK
				return;

			wglDeleteContext(m_context);
			m_context = nullptr;
		}

	private:
		HGLRC m_context = nullptr;
		Profile m_profile;
		vec2<u8> m_min_version;
		u8 m_swap_interval = 0;
	};
}
