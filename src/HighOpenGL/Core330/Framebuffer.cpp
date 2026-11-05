module;

#include <Core/Defines.hpp>

#include <OpenGL/Core330/gl.h>

#include <tuple>
#include <utility>

export module Nyaanwork.HighOpenGL.Core330:Framebuffer;
import :Context;
import :Texture2D;

import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Ptr;
import Nyaanwork.HighOpenGL.Exception;

namespace Nyaanwork::HighOpenGL::Core330
{
	template<typename... ColorAttachments>
	struct FramebufferInfo
	{
		std::tuple<PtrContainer<Texture2D<ColorAttachments>>...> colors;
		PtrContainer<Texture2D<DepthStencil>> depth = nullptr;
	};

	template<>
	struct FramebufferInfo<>
	{
		PtrContainer<Texture2D<DepthStencil>> depth = nullptr;
	};
}

export namespace Nyaanwork::HighOpenGL::Core330
{
	class Framebuffer final
	{
	public:
		template<typename... ColorAttachments>
		using Info = FramebufferInfo<ColorAttachments...>;

		Framebuffer() = default;

		template<typename... ColorAttachments>
		Framebuffer(PtrContainer<Context> context, const Info<ColorAttachments...>& info)
			: m_context(context), m_bind(context->gl().BindFramebuffer)
		{
			auto& gl = context->gl();

			gl.GenFramebuffers(1, &m_id);
			bind();

			std::array<GLenum, sizeof...(ColorAttachments)> bind_buffers = {};
			if constexpr (sizeof...(ColorAttachments))
			{
				std::apply( // FIXME: find better way
					[&gl, &bind_buffers](const auto&... attachments)
					{
						usize i = 0;

						([&gl, &bb=bind_buffers, &i](const auto& attachment)
						{
							gl.FramebufferTexture2D(GL_FRAMEBUFFER,
													bb[i] = GL_COLOR_ATTACHMENT0+i,
													GL_TEXTURE_2D, attachment->get(), 0);
							++i;
							return 0;
						}(attachments) || ...);

					},
					info.colors
				);
			}

			if (info.depth)
			{
				gl.FramebufferTexture2D(GL_FRAMEBUFFER,
										GL_DEPTH_STENCIL_ATTACHMENT,
										GL_TEXTURE_2D, info.depth->get(), 0);
			}

			if constexpr (sizeof...(ColorAttachments))
				gl.DrawBuffers(bind_buffers.size(), bind_buffers.data());

			m_bind(GL_FRAMEBUFFER, 0);
		}

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;

		Framebuffer(Framebuffer&& oth) noexcept
			: m_context(std::move(oth.m_context)),
			  m_bind(std::exchange(oth.m_bind, nullptr)),
			  m_id(std::exchange(oth.m_id, 0)) {}

		Framebuffer& operator=(Framebuffer&& oth) noexcept
		{
			if (&oth != this)
			{
				destroy();

				m_context = std::move(oth.m_context);
				m_bind = std::exchange(oth.m_bind, nullptr);
				m_id = std::exchange(oth.m_id, 0);
			}

			return *this;
		}

		~Framebuffer()
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

		void bind() noexcept(!is_debug)
		{
			if constexpr (is_debug)
			{
				auto stat = m_context->gl().CheckFramebufferStatus(GL_FRAMEBUFFER);
				if (stat != GL_FRAMEBUFFER_COMPLETE) NYAAN_UNLIK
					throw Error::Exception(Error::Code::framebuffer_not_complete);
			}

			m_bind(GL_FRAMEBUFFER, get());
		}

	private:
		void destroy() noexcept
		{
			if (valid())
				m_context->gl().DeleteFramebuffers(1, &m_id);
			m_id = 0;
		}

		PtrContainer<Context> m_context;
		decltype(GladGLContext::BindFramebuffer) m_bind;

		GLuint m_id = 0;
	};
}
