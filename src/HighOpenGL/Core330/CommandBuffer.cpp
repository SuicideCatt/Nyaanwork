module;

#include <Core/Defines.hpp>

#include <OpenGL/Core330/gl.h>

#include <unordered_map>

export module Nyaanwork.HighOpenGL.Core330:CommandBuffer;
import :Checks;
import :Context;
import :Framebuffer;
import :Mesh;
import :Shader;
import :Texture2D;
import :UniformBuffer;

import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.HighOpenGL.Exception;
import Nyaanwork.WindowSystem.Backends.Base;
import Nyaanwork.WindowSystem.OpenGL;

namespace Nyaanwork
{
	namespace HighOpenGL::Core330
	{
		enum class ClearFlags : u8
		{
			none,
			color = 1 << 0,
			depth = 1 << 1, stencil = 1 << 2,
		};

		enum class StencilFace : u8
		{
			none,
			front = 1 << 0,
			back = 1 << 1,
		};
	}

	export template<>
	struct FlagTraits<HighOpenGL::Core330::ClearFlags>
		: FlagInfo<HighOpenGL::Core330::ClearFlags>
	{
		static constexpr bool is_bit_type = true;

		static constexpr Flags depth_stencil = Type::depth | Type::stencil;
		static constexpr Flags all = Type::color | depth_stencil;
	};

	export template<>
	struct FlagTraits<HighOpenGL::Core330::StencilFace>
		: FlagInfo<HighOpenGL::Core330::StencilFace>
	{
		static constexpr bool is_bit_type = true;

		static constexpr Flags front_and_back = Type::front | Type::back;
		static constexpr Flags all = front_and_back;
	};
}

export namespace Nyaanwork::HighOpenGL::Core330
{
	class CommandBuffer
	{
	public:
		CommandBuffer(Ptr<WindowSystem::OpenGL::Surface> surface,
					  PtrContainer<Context> context) noexcept
			: m_surface(surface), m_context(context), m_gl(context->gl()) {}

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;

		CommandBuffer(CommandBuffer&&) = delete;
		CommandBuffer& operator=(CommandBuffer&&) = delete;

		struct Clear
		{
			using Flags = ClearFlags;

			Opt<Utils::Flags<Flags>> flags = nullopt;

			Opt<Asset::Pixels::RGBA> color = nullopt;
			Opt<GLclampd> depth = nullopt;
			Opt<GLint> stencil = nullopt;
		};

		using StencilFace = Core330::StencilFace;

		struct Stencil
		{
			Opt<DepthStencilCompare> compare = nullopt;
			Opt<u8> reference = nullopt;
			Opt<u8> compare_mask = nullopt;
		};

		void clear_config(const Clear& clear) noexcept
		{
			auto& gl = *m_gl;

			if (clear.flags)
			{
				auto& flags = *clear.flags;

				m_clear.flags = 0;
				if (flags & Clear::Flags::color)
					m_clear.flags |= GL_COLOR_BUFFER_BIT;
				if (flags & Clear::Flags::depth)
					m_clear.flags |= GL_DEPTH_BUFFER_BIT;
				if (flags & Clear::Flags::stencil)
					m_clear.flags |= GL_STENCIL_BUFFER_BIT;
			}

			if (clear.color && *clear.color != m_clear.config.color)
			{
				m_clear.config.color = *clear.color;
				auto [r, g, b, a] = static_cast<vec4<GLclampf>>(m_clear.config.color);
				gl.ClearColor(r, g, b, a);
			}

			if (clear.depth && *clear.depth != m_clear.config.depth)
				gl.ClearDepth(m_clear.config.depth = *clear.depth);

			if (clear.stencil && *clear.stencil != m_clear.config.stencil)
				gl.ClearStencil(m_clear.config.stencil = *clear.stencil);
		}

		void clear() noexcept
			{ m_gl->Clear(m_clear.flags); }

		void viewport(WindowSystem::Backends::Base::Window::Resolution resolution,
					  WindowSystem::Backends::Base::Window::Position position = {0, 0}) noexcept
		{
			if (position == m_viewport.position && resolution == m_viewport.resolution)
				return;

			m_viewport.position = position;
			m_viewport.resolution = resolution;

			m_gl->Viewport(position.x, position.y, resolution.x, resolution.y);
		}

		void swap()
			{ m_surface->swap_buffers(); }

		void reset() noexcept
		{
			bind_framebuffer();
			bind_shader_program();
			bind_uniform_buffers();
			bind_textures();
			bind_mesh();
		}

		void bind_framebuffer(PtrContainer<Framebuffer> framebuffer) noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!framebuffer) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			auto new_framebuffer = framebuffer->get();
			if (new_framebuffer != m_framebuffer)
			{
				framebuffer->bind();
				m_framebuffer = new_framebuffer;
			}
		}

		void bind_framebuffer() noexcept
		{
			if (m_framebuffer)
				m_gl->BindFramebuffer(GL_FRAMEBUFFER, m_framebuffer = 0);
		}

		void bind_shader_program(PtrContainer<Shader::Program> program) noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!program) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			if (program.get() == m_shader.program.get())
				return;

			m_shader.program = program;
			auto& s = *m_shader.program;
			s.bind<false>();

			auto& gl = *m_gl;

			auto new_layout = s.layout();

			auto update =
				[&old=m_shader.layout, &new_=new_layout]
					(auto&& proj, auto&& functor, auto&&... args)
				{
					if (old.*proj == new_.*proj)
						return;

					functor(new_.*proj, args...);

					old.*proj = new_.*proj;
				};

			auto& enable = gl.Enable;
			auto& disable = gl.Disable;

			auto enable_disable = [&enable, &disable](bool mode, GLenum who)
			{
				if (mode)
					enable(who);
				else
					disable(who);
			};

			using L = Layout;

			{
				auto blend_new = static_cast<bool>(new_layout.blend);

				if (m_shader.blend != blend_new)
					enable_disable(m_shader.blend = blend_new, GL_BLEND);

				if (blend_new)
				{
					auto& old = *m_shader.layout.blend;
					auto& new_ = *new_layout.blend;

					auto compare_factors = [](const auto& a, const auto& b)
					{
						auto& [a_src, a_dst] = a.factor;
						auto& [b_src, b_dst] = b.factor;

						return a_src != b_src || a_dst != b_dst;
					};

					if (compare_factors(old.color, new_.color)
						|| compare_factors(old.alpha, new_.alpha))
					{
						gl.BlendFuncSeparate(
							to_gl(new_.color.factor.src), to_gl(new_.color.factor.dst),
							to_gl(new_.alpha.factor.src), to_gl(new_.alpha.factor.dst)
						);
					}

					if (old.color.operation != new_.color.operation
						|| old.alpha.operation != new_.alpha.operation)
					{
						gl.BlendEquationSeparate(
							to_gl(new_.color.operation), to_gl(new_.alpha.operation)
						);
					}

					old = new_;
				}
			}

			{
				auto depth_new = static_cast<bool>(new_layout.depth);

				if (m_shader.depth != depth_new)
					enable_disable(m_shader.depth = depth_new, GL_DEPTH_TEST);

				if (depth_new)
				{
					auto& old = *m_shader.layout.depth;
					auto& new_ = *new_layout.depth;

					if (old.compare != new_.compare)
						gl.DepthFunc(to_gl(new_.compare));

					if (old.min != new_.min || old.max != new_.max)
						gl.DepthRange(new_.min, new_.max);

					old = new_;
				}
			}

			{
				auto stencil_new = static_cast<bool>(new_layout.stencil);

				if (m_shader.stencil != stencil_new)
					enable_disable(m_shader.stencil = stencil_new, GL_STENCIL_TEST);

				if (stencil_new)
				{
					auto& old = *m_shader.layout.stencil;
					auto& new_ = *new_layout.stencil;

					auto set_for_face =
						[&old, &new_, &gl](GLenum face, auto&& proj, const auto& cache)
						{
							auto& old_ops = old.*proj;
							auto& new_ops = new_.*proj;

							if (old_ops.fail != new_ops.fail
								|| old_ops.pass != new_ops.pass
								|| old_ops.depth_fail != new_ops.depth_fail)
							{
								gl.StencilOpSeparate(
									face,
									to_gl(new_ops.fail), to_gl(new_ops.depth_fail),
									to_gl(new_ops.pass)
								);
							}
						};

					set_for_face(GL_FRONT, &L::Stencil::front, m_stencil.front.reference);
					set_for_face(GL_BACK, &L::Stencil::back, m_stencil.back.reference);

					old = new_;
				}
			}

			update(&L::polygon_mode, [&gl, &new_=new_layout, this](PolygonMode mode)
			{
				gl.PolygonMode(GL_FRONT_AND_BACK, to_gl(mode));
				switch (mode)
				{
				case PolygonMode::line:
					if (m_shader.line_width != new_.line_width)
						gl.LineWidth(m_shader.line_width = new_.line_width);
					break;
				case PolygonMode::point:
					if (m_shader.point_size != new_.point_size)
						gl.PointSize(m_shader.point_size = new_.point_size);
					break;

				default:
					break;
				}
			});

			update(&L::front_face, [&gl](FrontFace face)
				{ gl.FrontFace(to_gl(face)); });

			update(&L::cull_mode, [&gl, &enable, &disable, this](CullMode mode)
			{
				switch (mode)
				{
				case CullMode::none:
					disable(GL_CULL_FACE);
					break;
				default:
					enable(GL_CULL_FACE);
					gl.CullFace(to_gl(mode));
					break;
				}
			});

			m_shader.layout.vertex_descriptor = new_layout.vertex_descriptor;

			m_stencil.front.reference = 0;
			m_stencil.back.reference = 0;
		}

		void bind_shader_program() noexcept
		{
			if (m_shader.program)
			{
				m_gl->UseProgram(0);
				m_shader.program = nullptr;

				m_stencil.front.reference = 0;
				m_stencil.back.reference = 0;
			}
		}

		Shader::Program& operator*() noexcept(!is_debug)
			{ return *m_shader.program; }

		Shader::Program* operator->() noexcept(!is_debug)
			{ return m_shader.program.ptr(); }

		void blend_constant(Asset::Pixels::RGBA color)
		{
			if (color != m_blend_constant)
			{
				auto [r, g, b, a] = static_cast<vec4<GLfloat>>(m_blend_constant = color);
				m_gl->BlendColor(r, g, b, a);
			}
		}

		void stencil_mask(Flags<StencilFace> faces, u8 mask)
		{
			using T = FlagTraits<StencilFace>;
			using M = T::MaskType;

			auto& set = m_gl->StencilMaskSeparate;

			switch (static_cast<M>(faces))
			{
			case T::cast(StencilFace::front):
				if (m_stencil.front.write_mask != mask)
					set(GL_FRONT, m_stencil.front.write_mask = mask);
				break;

			case T::cast(StencilFace::back):
				if (m_stencil.back.write_mask != mask)
					set(GL_BACK, m_stencil.back.write_mask = mask);
				break;

			case static_cast<M>(T::front_and_back):
				if (m_stencil.front.write_mask != mask
					|| m_stencil.back.write_mask != mask)
				{
					m_stencil.front.write_mask = m_stencil.back.write_mask = mask;
					set(GL_FRONT_AND_BACK, mask);
				}
				break;
			}
		}

		void stencil(Flags<StencilFace> faces, const Stencil& stencil)
		{
			if (!m_shader.stencil) NYAAN_UNLIK
				return;

			auto& cache = m_stencil.front;

			auto compare = [&stencil](const auto& cache)
			{
				auto compare = [](const auto& opt, const auto& cache)
					{ return opt && *opt != cache; };

				return compare(stencil.compare, cache.compare)
					|| compare(stencil.reference, cache.reference)
					|| compare(stencil.compare_mask, cache.compare_mask);
			};

			auto copy_to = [&stencil](auto& cache)
			{
				auto copy_if = [](const auto& opt, auto& cache)
				{
					if (opt)
						cache = *opt;
				};

				copy_if(stencil.compare, cache.compare);
				copy_if(stencil.reference, cache.reference);
				copy_if(stencil.compare_mask, cache.compare_mask);
			};

			auto set =
				[&set=m_gl->StencilFuncSeparate](GLenum faces, const auto& cache)
				{
					set(
						faces, to_gl(cache.compare),
						cache.reference, cache.compare_mask
					);
				};

			using T = FlagTraits<StencilFace>;
			using M = T::MaskType;
			switch (static_cast<M>(faces))
			{
			case T::cast(StencilFace::front):
				if (compare(m_stencil.front))
				{
					copy_to(m_stencil.front);
					set(GL_FRONT, m_stencil.front);
				}
				break;

			case T::cast(StencilFace::back):
				if (compare(m_stencil.back))
				{
					copy_to(m_stencil.back);
					set(GL_BACK, m_stencil.back);
				}
				break;

			case static_cast<M>(T::front_and_back):
				if (compare(m_stencil.front) || compare(m_stencil.back))
				{
					copy_to(m_stencil.front);
					copy_to(m_stencil.back);
					set(GL_FRONT_AND_BACK, m_stencil.front);
				}
				break;
			}
		}

		template<typename Type>
		void bind_uniform_buffer(PtrContainer<UniformBuffer<Type>> buffer,
								 u8 slot) noexcept(noexcept(buffer->bind(slot)))
		{
			if constexpr (is_debug)
				if (!buffer) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			auto new_buffer = buffer->get();

			auto itr = m_uniform_buffers.find(slot);
			if (itr == m_uniform_buffers.end())
				itr = m_uniform_buffers.emplace(slot, 0).first;

			auto& old_buffer = itr->second;
			if (old_buffer != new_buffer)
			{
				buffer->bind(slot);
				old_buffer = new_buffer;
			}
		}

		template<typename Type>
		void bind_uniform_buffer(UniformBuffer<Type>& buffer,
								 u8 slot) noexcept(noexcept(buffer.bind(slot)))
			{ bind_uniform_buffer(PtrContainer(buffer), slot); }

		template<typename Type>
		void bind_uniform_buffer(UniformBuffer<Type>* buffer,
								 u8 slot) noexcept(noexcept(buffer->bind(slot)))
			{ bind_uniform_buffer(PtrContainer(buffer), slot); }

		void bind_uniform_buffer(u8 slot)
				noexcept(!Context::dynamic_limits_mode && !is_debug)
		{
			check_uniform_slot(*m_context, slot);

			auto itr = m_uniform_buffers.find(slot);
			if (itr != m_uniform_buffers.end())
			{
				auto& old_buffer = itr->second;
				if (old_buffer)
				{
					m_gl->BindBufferBase(GL_UNIFORM_BUFFER, slot, 0);
					old_buffer = 0;
				}
			}
		}

		void bind_uniform_buffers() noexcept
		{
			for (const auto& [slot, id] : m_uniform_buffers)
				if (id)
					bind_uniform_buffer(slot);
		}

		template<typename Type>
		void bind_texture_2d(PtrContainer<Texture2D<Type>> texture, u8 slot)
				noexcept(noexcept(texture->template bind<true>(slot)))
		{
			if constexpr (is_debug)
				if (!texture) NYAAN_UNLIK
					throw Error::Exception("texture", Error::Code::not_valid);

			GLuint new_texture = texture->get();

			auto itr = m_texture.slots.find(slot);
			if (itr == m_texture.slots.end())
				itr = m_texture.slots.emplace(slot, 0).first;

			auto& old_texture = itr->second;
			if (new_texture != old_texture)
			{
				if (m_texture.active != slot)
					texture->template bind<true>(slot);
				else
					texture->template bind<false>(slot);

				m_texture.active = slot;
				old_texture = new_texture;
			}
		}

		template<typename Type>
		void bind_texture_2d(Texture2D<Type>& texture, u8 slot)
				noexcept(noexcept(texture.template bind<true>(slot)))
			{ bind_texture_2d(PtrContainer(texture), slot); }

		template<typename Type>
		void bind_texture_2d(Texture2D<Type>* texture, u8 slot)
				noexcept(noexcept(texture->template bind<true>(slot)))
			{ bind_texture_2d(PtrContainer(texture), slot); }

		void bind_texture_2d(u8 slot) noexcept(!Context::dynamic_limits_mode && !is_debug)
		{
			check_texture_slot(*m_context, slot);

			auto& gl = *m_gl;

			auto itr = m_texture.slots.find(slot);
			if (itr != m_texture.slots.end())
			{
				if (m_texture.active != slot)
					gl.ActiveTexture(GL_TEXTURE0 + slot);
				m_texture.active = slot;

				gl.BindTexture(GL_TEXTURE_2D, itr->second = 0);
			}
		}

		void bind_textures() noexcept
		{
			for (const auto& [i, id] : m_texture.slots)
				if (id)
					bind_texture_2d(i);
		}

		template<typename Vertex, typename Index, bool dynamic>
		void bind_mesh(PtrContainer<MeshWithIndex<Vertex, Index, dynamic>> mesh)
				noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!mesh) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			GLuint new_mesh = mesh->get();

			if (m_mesh.mesh != new_mesh)
			{
				if constexpr (is_debug)
					if (m_shader.layout.vertex_descriptor != mesh->vertex_descriptor())
						throw Error::Exception("vertex_descriptor", Error::Code::not_valid);

				mesh->bind();
				m_mesh.mesh = new_mesh;
				m_mesh.index_type = to_gl<Index>();
				m_mesh.topology = to_gl(mesh->topology());
				m_mesh.count = mesh->indices().second;
			}
		}

		template<typename Vertex, typename Index, bool dynamic>
		void bind_mesh(MeshWithIndex<Vertex, Index, dynamic>& mesh) noexcept(!is_debug)
			{ bind_mesh(PtrContainer(mesh)); }

		template<typename Vertex, typename Index, bool dynamic>
		void bind_mesh(MeshWithIndex<Vertex, Index, dynamic>* mesh) noexcept(!is_debug)
			{ bind_mesh(PtrContainer(mesh)); }

		template<typename Vertex, bool dynamic>
		void bind_mesh(PtrContainer<MeshWithoutIndex<Vertex,
													 dynamic>> mesh) noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!mesh) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			GLuint new_mesh = mesh->get();

			if (m_mesh.mesh != new_mesh)
			{
				if constexpr (is_debug)
					if (m_shader.layout.vertex_descriptor != mesh->vertex_descriptor())
						throw Error::Exception("vertex_descriptor", Error::Code::not_valid);

				mesh->bind();
				m_mesh.mesh = new_mesh;
				m_mesh.index_type = nullopt;
				m_mesh.topology = to_gl(mesh->topology());
				m_mesh.count = mesh->vertices().second;
			}
		}

		template<typename Vertex, bool dynamic>
		void bind_mesh(MeshWithoutIndex<Vertex, dynamic>& mesh) noexcept(!is_debug)
			{ bind_mesh(PtrContainer(mesh)); }

		template<typename Vertex, bool dynamic>
		void bind_mesh(MeshWithoutIndex<Vertex, dynamic>* mesh) noexcept(!is_debug)
			{ bind_mesh(PtrContainer(mesh)); }

		void bind_mesh() noexcept
		{
			if (m_mesh.mesh)
			{
				m_gl->BindVertexArray(0);
				m_mesh = {};
			}
		}

		void draw(u32 instances = 1, Opt<usize> count = nullopt) noexcept(!is_debug)
		{
			if constexpr (is_debug)
			{
				if (!m_shader.program)
					throw Error::Exception("shader_program", Error::Code::not_valid);
				if (m_mesh.mesh == 0)
					throw Error::Exception("mesh", Error::Code::not_valid);
			}

			auto& gl = *m_gl;

			if (m_mesh.index_type)
			{
				gl.DrawElementsInstanced(
					m_mesh.topology,
					count.value_or(m_mesh.count), *m_mesh.index_type,
					nullptr, instances
				);
			}
			else
			{
				gl.DrawArraysInstanced(
					m_mesh.topology, 0, count.value_or(m_mesh.count), instances
				);
			}
		}

	private:
		Ptr<WindowSystem::OpenGL::Surface> m_surface;
		PtrContainer<Context> m_context;
		PtrContainer<const GladGLContext> m_gl;

		struct
		{
			GLbitfield flags = 0;
			struct
			{
				Asset::Pixels::RGBA color = Asset::Colors<Asset::Pixels::RGBA>::null;
				GLint stencil = 0;
				GLclampd depth = 1.0;
			} config;
		} m_clear;

		struct
		{
			WindowSystem::Backends::Base::Window::Position position = {0, 0};
			WindowSystem::Backends::Base::Window::Resolution resolution = {0, 0};
		} m_viewport;

		Asset::Pixels::RGBA m_blend_constant = Asset::Colors<Asset::Pixels::RGBA>::null;
		GLuint m_framebuffer = 0;

		struct
		{
			PtrContainer<Shader::Program> program;
			Layout layout = {
				.blend = Layout::Blend{},
				.depth = Layout::Depth{},
				.stencil = Layout::Stencil{},
			};

			f32 line_width = 1.f;
			f32 point_size = 1.f;

			bool blend = false;
			bool depth = false;
			bool stencil = false;
		} m_shader;

		struct
		{
			struct {
				DepthStencilCompare compare = DepthStencilCompare::always;
				u8 reference = 0;
				u8 compare_mask = 0xFF;
				u8 write_mask = 0xFF;
			} front, back;
		} m_stencil;

		std::unordered_map<u8, GLuint> m_uniform_buffers;

		struct
		{
			std::unordered_map<u8, GLuint> slots;
			u8 active = 0;
		} m_texture;

		struct
		{
			GLuint mesh = 0;
			Opt<GLenum> index_type = nullopt;
			GLenum topology = GL_TRIANGLES;
			usize count = 0;
		} m_mesh;
	};
}
