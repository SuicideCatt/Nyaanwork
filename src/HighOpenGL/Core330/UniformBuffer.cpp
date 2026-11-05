module;

#include <Core/Defines.hpp>
#include <Nyaanwork/HighOpenGL.hpp>

#include <OpenGL/Core330/gl.h>

#include <format>
#include <utility>

export module Nyaanwork.HighOpenGL.Core330:UniformBuffer;
import :Checks;
import :Context;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Ptr;
import Nyaanwork.HighOpenGL.Exception;

export namespace Nyaanwork::HighOpenGL::Core330
{
	template<typename _Type>
	#ifndef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
	requires (sizeof(_Type) <= Context::static_limits.uniform_block_size)
	#endif
	class UniformBuffer
	{
	public:
		using Type = _Type;

		UniformBuffer() = default;

		UniformBuffer(PtrContainer<Context> context) noexcept(!Context::dynamic_limits_mode)
			: m_context(context), m_gl(context->gl())
		{
			if constexpr (Context::dynamic_limits_mode)
			{
				auto size = m_context->limits().uniform_block_size;
				if (!(sizeof(Type) <= size))
				{
					auto msg = std::format("uniform_block_size: !({} <= {})",
											sizeof(Type), size);
					throw Error::Exception(msg, Error::Code::limit_exceeded);
				}
			}

			auto& gl = *m_gl;

			gl.GenBuffers(1, &m_id);
			gl.BindBuffer(GL_UNIFORM_BUFFER, get());
			gl.BufferData(GL_UNIFORM_BUFFER, sizeof(Type), nullptr, GL_STATIC_DRAW);
			gl.BindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;

		UniformBuffer(UniformBuffer&& oth) noexcept
			: m_context(std::move(oth.m_context)),
			  m_gl(std::move(oth.m_gl)),
			  m_id(std::exchange(oth.m_id, 0)),
			  m_data(std::move(oth.m_data)) {}

		UniformBuffer& operator=(UniformBuffer&& oth) noexcept
		{
			if (&oth != this)
			{
				destroy();

				m_context = std::move(oth.m_context);
				m_gl = std::move(oth.m_gl);
				m_id = std::exchange(oth.m_id, 0);
				m_data = std::move(oth.m_data);
			}

			return *this;
		}

		~UniformBuffer()
			{ destroy(); }

		PtrContainer<Context> context() NYAAN_CNOEX
			{ return m_context; }

		bool valid() NYAAN_CNOEX
			{ return m_id != 0 && m_id != static_cast<GLuint>(-1); }

		GLuint get() NYAAN_CNOEX(!is_debug)
		{
			if constexpr (is_debug)
				if (!valid()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			return m_id;
		}

		Type* operator->() noexcept
			{ return &m_data; }

		Type& operator*() noexcept
			{ return m_data; }

		const Type* operator->() NYAAN_CNOEX(!is_debug)
			{ return &m_data; }

		const Type& operator*() NYAAN_CNOEX(!is_debug)
			{ return m_data; }

		void sync() noexcept(!is_debug)
		{
			auto& gl = *m_gl;

			gl.BindBuffer(GL_UNIFORM_BUFFER, get());
			gl.BufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Type), &m_data);
			gl.BindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void bind(u8 slot) noexcept(!Context::dynamic_limits_mode && !is_debug)
		{
			if constexpr (Context::dynamic_limits_mode || is_debug)
				check_unifrom_slot(*m_context, slot);

			m_gl->BindBufferBase(GL_UNIFORM_BUFFER, slot, get());
		}

	private:
		void destroy() noexcept
		{
			if (valid())
				m_gl->DeleteBuffers(1, &m_id);

			m_data = Type();
			m_id = 0;
		}

		PtrContainer<Context> m_context;
		PtrContainer<const GladGLContext> m_gl;

		GLuint m_id = 0;
		Type m_data;
	};
}
