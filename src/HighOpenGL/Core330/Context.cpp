module;

#include <Core/Defines.hpp>
#include <Nyaanwork/HighOpenGL.hpp>

#include <OpenGL/Core330/gl.h>

#include <compare>

export module Nyaanwork.HighOpenGL.Core330:Context;

import Nyaanwork.Core.Types;
import Nyaanwork.HighOpenGL.Exception;
import Nyaanwork.WindowSystem.OpenGL;

export namespace Nyaanwork::HighOpenGL::Core330
{
	class Context final
	{
	public:
		#ifdef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
		static constexpr bool dynamic_limits_mode = true;
		#else
		static constexpr bool dynamic_limits_mode = false;
		#endif

		struct Limits
		{
			constexpr auto operator<=>(const Limits&) NYAAN_CNOEX = default;

			GLint texture_slots;
			GLint texture_size;

			GLint uniform_buffer_bindings;
			GLint uniform_block_size;
		};

		#ifndef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
		static constexpr Limits static_limits = {
			.texture_slots = NYAANWORK_HIGH_OPENGL_CORE330_TEXTURE_SLOTS,
			.texture_size = NYAANWORK_HIGH_OPENGL_CORE330_TEXTURE_SIZE,

			.uniform_buffer_bindings =
				NYAANWORK_HIGH_OPENGL_CORE330_UNIFORM_BUFFER_BINDINGS,
			.uniform_block_size = NYAANWORK_HIGH_OPENGL_CORE330_UNIFORM_BLOCK_SIZE,
		};
		#endif

		Context(Ptr<WindowSystem::OpenGL::Instance> instance)
			: m_context(instance, WindowSystem::OpenGL::Context::Profile::core,
						{3, 3}) {}

		Ptr<WindowSystem::OpenGL::Instance> instance() NYAAN_CNOEX
			{ return m_context.instance(); }

		bool is_current() NYAAN_CNOEX
			{ return m_context.is_current(); }
		void make_current(Ptr<WindowSystem::OpenGL::Surface> surface)
		{
			m_context.make_current(surface);

			if (valid())
				return;

			using WindowSystem::OpenGL::Context;
			if (!gladLoadGLContext(&m_glad, Context::get_proc_address))
				throw Error::Exception(Error::Code::fail_load_glad);

			auto& gl = this->gl();

			auto get_lim = [&get=gl.GetIntegerv](GLenum gl, auto& lim)
				{ get(gl, &lim); };

			get_lim(GL_MAX_TEXTURE_IMAGE_UNITS, m_limits.texture_slots);
			get_lim(GL_MAX_TEXTURE_SIZE, m_limits.texture_size);
			get_lim(GL_MAX_UNIFORM_BUFFER_BINDINGS, m_limits.uniform_buffer_bindings);
			get_lim(GL_MAX_UNIFORM_BLOCK_SIZE, m_limits.uniform_block_size);

			#ifndef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
			static constexpr Limits limits = static_limits;
			if (static_limits > m_limits)
				throw Error::Exception(Error::Code::limits_not_supported);
			#endif

			m_vendor = reinterpret_cast<const char*>(gl.GetString(GL_VENDOR));
			m_renderer = reinterpret_cast<const char*>(gl.GetString(GL_RENDERER));
		}

		bool valid() NYAAN_CNOEX
			{ return m_glad.VERSION_3_3; }

		u8 swap_interval() NYAAN_CNOEX
			{ return  m_context.swap_interval(); }
		void swap_interval(u8 interval) noexcept
			{ m_context.swap_interval(interval); }

		const GladGLContext& gl() NYAAN_CNOEX
			{ return m_glad; }
		const GladGLContext* operator->() NYAAN_CNOEX
			{ return &gl(); }

		#ifdef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
		Limits limits() NYAAN_CNOEX
			{ return m_limits; }
		#else
		static constexpr Limits limits() noexcept
			{ return static_limits; }

		Limits dynamic_limits() NYAAN_CNOEX
			{ return m_limits; }
		#endif

		Str vendor() NYAAN_CNOEX
			{ return m_vendor; }
		Str renderer() NYAAN_CNOEX
			{ return m_renderer; }

	private:
		WindowSystem::OpenGL::Context m_context;
		GladGLContext m_glad = {};
		Limits m_limits = {};
		Str m_vendor;
		Str m_renderer;
	};
}
