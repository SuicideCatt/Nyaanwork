module;

#include <Core/Defines.hpp>

#include <OpenGL/Core330/gl.h>

#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

export module Nyaanwork.HighOpenGL.Core330:Shader;
import :Checks;
import :Context;
import :VertexDescriptor;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Ptr;
import Nyaanwork.HighOpenGL.Exception;

namespace Nyaanwork::HighOpenGL::Core330::Shader::Meta // Nyaanwork.Core.Utils.Signals:List
{
	template<typename _Front, typename _PopFront>
	struct ListNode
	{
		using PopFront = _PopFront; // next ListNode
		using Front = _Front;
	};

	template<typename Front, typename... Others>
	struct List : ListNode<Front, List<Others...>> {};

	template<typename Front>
	struct List<Front> : ListNode<Front, void> {};

	// loop
	template<bool error, typename Type, typename List>
	struct Find : Find<error, Type, typename List::PopFront> {};

	// if requires pass loop ends
	template<bool error, typename _Type, typename List>
	requires (std::same_as<typename List::Front::Type, _Type>)
	struct Find<error, _Type, List>
	{
		using Type = List::Front;
	};

	template<typename Type>
	struct Find<true, Type, void>
	{
		static_assert(false, "Cannot find the UniformTableElement");
	};

	template<typename _Type>
	struct Find<false, _Type, void>
	{
		using Type = void;
	};

	template<typename Type, typename List>
	concept contain_type =
		!std::is_void_v<typename Find<false, Type, List>::Type>;

	template<typename _Type, auto _function, typename _UniformType = _Type>
	struct UniformTableElement
	{
		using Type = _Type;
		static constexpr auto function = _function;
		using UniformType = _UniformType;
	};

	template<typename... Elements>
	struct UniformTable
	{
		using List = Meta::List<Elements...>;

		template<typename Type>
		using Find = Meta::Find<true, Type, List>::Type;
	};
}

namespace Nyaanwork::HighOpenGL::Core330::Shader
{
	using UniformTable = Meta::UniformTable<
		Meta::UniformTableElement<i32, &GladGLContext::Uniform1iv>,
		Meta::UniformTableElement<u32, &GladGLContext::Uniform1uiv>,

		Meta::UniformTableElement<f32, &GladGLContext::Uniform1fv>,
		Meta::UniformTableElement<vec2<f32>, &GladGLContext::Uniform2fv, f32>,
		Meta::UniformTableElement<vec3<f32>, &GladGLContext::Uniform3fv, f32>,
		Meta::UniformTableElement<vec4<f32>, &GladGLContext::Uniform4fv, f32>
	>;

	template<typename Type>
	concept uniform_table_contain_type = Meta::contain_type<Type, UniformTable::List>;
}

export namespace Nyaanwork::HighOpenGL::Core330
{
	enum class FrontFace : u8
	{
		counter_clockwise, clockwise,
	};

	enum class PolygonMode : u8
	{
		fill, line, point
	};

	enum class CullMode : u8
	{
		none, front, back, front_and_back
	};

	enum class DepthStencilCompare : u8
	{
		never, always,
		less, greater, equal,
		less_equal, greater_equal, not_equal
	};

	enum class StencilOp : u8
	{
		keep, zero, replace,
		increment, increment_wrap,
		decrement, decrement_wrap,
		invert,
	};

	enum class BlendFactor : u8
	{
		zero, one,

		src_color, one_minus_src_color,
		dst_color, one_minus_dst_color,

		src_alpha, one_minus_src_alpha,
		dst_alpha, one_minus_dst_alpha,

		constant_color, one_minus_constant_color,
		constant_alpha, one_minus_constant_alpha,

		src1_color, one_minus_src1_color,
		src1_alpha, one_minus_src1_alpha,
	};

	enum class BlendOp : u8
	{
		add,
		subtract, reverse_subtract,
		min, max
	};

	struct Layout
	{
		struct Blend
		{
			struct
			{
				struct {
					BlendFactor src = BlendFactor::one;
					BlendFactor dst = BlendFactor::zero;
				} factor;
				BlendOp operation = BlendOp::add;
			} color, alpha;
		};

		struct Depth
		{
			DepthStencilCompare compare = DepthStencilCompare::less;

			GLclampd min = 0.f;
			GLclampd max = 1.f;
		};

		struct Stencil
		{
			struct
			{
				StencilOp fail = StencilOp::keep;
				StencilOp pass = StencilOp::keep;
				StencilOp depth_fail = StencilOp::keep;
			} front, back;
		};

		VertexDescriptor vertex_descriptor;

		union
		{
			f32 line_width = 1.f;
			f32 point_size;
		};
		PolygonMode polygon_mode = PolygonMode::fill;
		FrontFace front_face = FrontFace::counter_clockwise;
		CullMode cull_mode = CullMode::none;

		Opt<Blend> blend = nullopt;
		Opt<Depth> depth = nullopt;
		Opt<Stencil> stencil = nullopt;
	};
}

namespace Nyaanwork::HighOpenGL::Core330
{
	constexpr GLenum to_gl(FrontFace face) noexcept
	{
		switch (face)
		{
		case FrontFace::counter_clockwise:
			return GL_CCW;
		case FrontFace::clockwise:
			return GL_CW;
		}
	}

	constexpr GLenum to_gl(PolygonMode mode) noexcept
	{
		switch (mode)
		{
		case PolygonMode::fill:
			return GL_FILL;
		case PolygonMode::line:
			return GL_LINE;
		case PolygonMode::point:
			return GL_POINT;
		}
	}

	constexpr GLenum to_gl(CullMode mode) noexcept
	{
		switch (mode)
		{
		case CullMode::none:
			return 0;
		case CullMode::front:
			return GL_FRONT;
		case CullMode::back:
			return GL_BACK;
		case CullMode::front_and_back:
			return GL_FRONT_AND_BACK;
		}
	}

	constexpr GLenum to_gl(DepthStencilCompare compare) noexcept
	{
		switch (compare)
		{
		case DepthStencilCompare::never:
			return GL_NEVER;
		case DepthStencilCompare::always:
			return GL_ALWAYS;

		case DepthStencilCompare::less:
			return GL_LESS;
		case DepthStencilCompare::greater:
			return GL_GREATER;
		case DepthStencilCompare::equal:
			return GL_EQUAL;

		case DepthStencilCompare::less_equal:
			return GL_LEQUAL;
		case DepthStencilCompare::greater_equal:
			return GL_GEQUAL;
		case DepthStencilCompare::not_equal:
			return GL_NOTEQUAL;
		}
	};

	constexpr GLenum to_gl(StencilOp op) noexcept
	{
		switch (op)
		{
		case StencilOp::keep:
			return GL_KEEP;
		case StencilOp::zero:
			return GL_ZERO;
		case StencilOp::replace:
			return GL_REPLACE;

		case StencilOp::increment:
			return GL_INCR;
		case StencilOp::increment_wrap:
			return GL_INCR_WRAP;

		case StencilOp::decrement:
			return GL_DECR;
		case StencilOp::decrement_wrap:
			return GL_DECR_WRAP;

		case StencilOp::invert:
			return GL_INVERT;
		}
	}

	constexpr GLenum to_gl(BlendFactor factor) noexcept
	{
		switch (factor)
		{
		case BlendFactor::zero:
			return GL_ZERO;
		case BlendFactor::one:
			return GL_ONE;

		case BlendFactor::src_color:
			return GL_SRC_COLOR;
		case BlendFactor::one_minus_src_color:
			return GL_ONE_MINUS_SRC_COLOR;
		case BlendFactor::dst_color:
			return GL_DST_COLOR;
		case BlendFactor::one_minus_dst_color:
			return GL_ONE_MINUS_DST_COLOR;

		case BlendFactor::src_alpha:
			return GL_SRC_ALPHA;
		case BlendFactor::one_minus_src_alpha:
			return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::dst_alpha:
			return GL_DST_ALPHA;
		case BlendFactor::one_minus_dst_alpha:
			return GL_ONE_MINUS_DST_ALPHA;

		case BlendFactor::constant_color:
			return GL_CONSTANT_COLOR;
		case BlendFactor::one_minus_constant_color:
			return GL_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::constant_alpha:
			return GL_CONSTANT_ALPHA;
		case BlendFactor::one_minus_constant_alpha:
			return GL_ONE_MINUS_CONSTANT_ALPHA;

		case BlendFactor::src1_color:
			return GL_SRC1_COLOR;
		case BlendFactor::one_minus_src1_color:
			return GL_ONE_MINUS_SRC1_COLOR;
		case BlendFactor::src1_alpha:
			return GL_SRC1_ALPHA;
		case BlendFactor::one_minus_src1_alpha:
			return GL_ONE_MINUS_SRC1_ALPHA;
		}
	};

	constexpr GLenum to_gl(BlendOp op) noexcept
	{
		switch (op)
		{
		case BlendOp::add:
			return GL_FUNC_ADD;
		case BlendOp::subtract:
			return GL_FUNC_SUBTRACT;
		case BlendOp::reverse_subtract:
			return GL_FUNC_REVERSE_SUBTRACT;
		case BlendOp::min:
			return GL_MIN;
		case BlendOp::max:
			return GL_MAX;
		}
	};
}

export namespace Nyaanwork::HighOpenGL::Core330::Shader
{
	class Module final
	{
	public:
		enum class Type : u8
		{
			vertex, fragment,
			geometry
		};

		struct Info
		{
			Type type = Type::vertex;
			std::span<char const> source;
		};

		Module() = default;

		Module(PtrContainer<Context> context, const Info& info)
			: m_context(context), m_type(info.type)
		{
			StrV type_str = "vertex";
			GLenum type = GL_VERTEX_SHADER;
			switch(m_type)
			{
			case Type::fragment:
				type_str = "fragment";
				type = GL_FRAGMENT_SHADER;
				break;
			case Type::geometry:
				type_str = "geometry";
				type = GL_GEOMETRY_SHADER;
				break;

			default:
				break;
			}

			auto& gl = context->gl();

			m_id = gl.CreateShader(type);

			const char* arr = info.source.data();
			GLint size = info.source.size();

			gl.ShaderSource(m_id, 1, &arr, &size);
			gl.CompileShader(m_id);

			GLint success;
			Str str(1024, ' ');
			gl.GetShaderiv(m_id, GL_COMPILE_STATUS, &success);
			if (!success) NYAAN_UNLIK
			{
				GLsizei s;
				gl.GetShaderInfoLog(m_id, 1024, &s, str.data());
				str.resize(s);

				destroy();
				throw Error::Exception(
					type_str,
					Error::Code::fail_compile_shader_module,
					str
				);
			}
		}

		Module(const Module&) = delete;
		Module& operator=(const Module&) = delete;

		Module(Module&& oth) noexcept
			: m_context(std::move(oth.m_context)),
			  m_id(std::exchange(oth.m_id, 0)),
			  m_type(std::exchange(oth.m_type, Type::vertex)) {}

		Module& operator=(Module&& oth) noexcept
		{
			if (&oth != this)
			{
				destroy();

				m_context = std::move(oth.m_context);
				m_id = std::exchange(oth.m_id, 0);
				m_type = std::exchange(oth.m_type, Type::vertex);
			}

			return *this;
		}

		~Module()
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

		Type type() NYAAN_CNOEX
			{ return m_type; }

	private:
		void destroy() noexcept
		{
			if (valid())
				m_context->gl().DeleteShader(m_id);
			m_id = 0;
		}

		PtrContainer<Context> m_context;
		GLuint m_id = 0;
		Type m_type = Type::vertex;
	};

	class Program final
	{
	public:
		struct Info
		{
			std::vector<PtrContainer<Module>> modules;
			Layout layout;
			struct {
				std::vector<Str> preload_locations;
				std::vector<Pair<Str, u8>> textures;
				std::vector<Pair<Str, u8>> buffers;
			} uniform_layout;
		};

		Program() = default;

		Program(PtrContainer<Context> context, const Info& info)
			: m_context(context), m_gl(context->gl()), m_layout(info.layout)
		{
			auto& gl = *m_gl;

			m_id = gl.CreateProgram();
			for (const auto& m : info.modules)
				gl.AttachShader(m_id, m->get());
			gl.LinkProgram(m_id);

			GLint success;
			Str str(1024, ' ');
			gl.GetProgramiv(m_id, GL_LINK_STATUS, &success);
			if (!success) NYAAN_UNLIK
			{
				GLsizei s;
				gl.GetProgramInfoLog(m_id, 1024, &s, str.data());
				str.resize(s);

				destroy();
				throw Error::Exception(Error::Code::fail_link_shader_program, str);
			}

			bind<false>();

			for (auto& name : info.uniform_layout.preload_locations)
				get_location(name);

			for (auto& [name, val] : info.uniform_layout.textures)
			{
				check_texture_slot(*context, val);
				set(name, static_cast<i32>(val));
			}

			for (auto& [name, val] : info.uniform_layout.buffers)
			{
				check_unifrom_slot(*context, val);
				auto loc = gl.GetUniformBlockIndex(get(), name.c_str());
				gl.UniformBlockBinding(get(), loc, val);
			}

			gl.UseProgram(0);
		}

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		Program(Program&& oth) noexcept
			: m_context(std::move(oth.m_context)),
			  m_gl(std::move(oth.m_gl)),
			  m_id(std::exchange(oth.m_id, 0)),
			  m_locations(std::move(oth.m_locations)),
			  m_layout(std::move(oth.m_layout)) {}

		Program& operator=(Program&& oth) noexcept
		{
			if (&oth != this)
			{
				destroy();

				m_context = std::move(oth.m_context);
				m_gl = std::move(oth.m_gl);
				m_id = std::exchange(oth.m_id, 0);
				m_locations = std::move(oth.m_locations);
				m_layout = std::exchange(oth.m_layout, {});
			}

			return *this;
		}

		~Program()
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

		Layout layout() NYAAN_CNOEX
			{ return m_layout; }

		template<bool update_layout = true>
		void bind() noexcept(!is_debug)
		{
			auto& gl = *m_gl;

			gl.UseProgram(get());

			if constexpr (update_layout)
			{
				auto& enable = gl.Enable;
				auto& disable = gl.Disable;

				auto enable_disable = [&enable, &disable](bool mode, GLenum who)
				{
					if (mode)
						enable(who);
					else
						disable(who);

					return mode;
				};

				if (enable_disable(m_layout.blend, GL_BLEND))
				{
					const auto& [color, alpha] = *m_layout.blend;

					gl.BlendFuncSeparate(
						to_gl(color.factor.src), to_gl(color.factor.dst),
						to_gl(alpha.factor.src), to_gl(alpha.factor.dst)
					);
					gl.BlendEquationSeparate(
						to_gl(color.operation), to_gl(alpha.operation)
					);
				}

				if (enable_disable(m_layout.depth, GL_DEPTH_TEST))
				{
					const auto& [compare, min, max] = *m_layout.depth;
					gl.DepthFunc(to_gl(compare));
					gl.DepthRange(min, max);
				}

				if (enable_disable(m_layout.stencil, GL_STENCIL_TEST))
				{
					const auto& [front, back] = *m_layout.stencil;

					auto set_for_face = [&gl](GLenum face, const auto& operations)
					{
						gl.StencilOpSeparate(
							face,
							to_gl(operations.fail), to_gl(operations.depth_fail),
							to_gl(operations.pass)
						);
					};

					set_for_face(GL_FRONT, front);
					set_for_face(GL_BACK, back);
				}

				gl.PolygonMode(GL_FRONT_AND_BACK, to_gl(m_layout.polygon_mode));
				switch (m_layout.polygon_mode)
				{
				case PolygonMode::line:
					gl.LineWidth(m_layout.line_width);
					break;
				case PolygonMode::point:
					gl.PointSize(m_layout.point_size);
					break;

				default:
					break;
				}

				gl.FrontFace(to_gl(m_layout.front_face));

				switch (m_layout.cull_mode)
				{
				case CullMode::none:
					disable(GL_CULL_FACE);
					break;
				default:
					enable(GL_CULL_FACE);
					gl.CullFace(to_gl(m_layout.cull_mode));
					break;
				}
			}
		}

		template<uniform_table_contain_type Type>
		void set(const Str& name, const Type& value) noexcept(!is_debug)
		{
			using E = UniformTable::Find<Type>;
			using Ptr = const E::UniformType*;

			const auto& func = ((m_gl.ptr())->*E::function);
			func(get_location(name), 1, reinterpret_cast<Ptr>(&value));
		}

		void set(const Str& name, bool value) noexcept(!is_debug)
			{ set(name, static_cast<i32>(value)); }

		template<auto len>
		requires (len == 3 || len == 4)
		void set(const Str& name, const Matrix<len, len, f32>& value,
				 bool transpose = false) noexcept(!is_debug)
		{
			static constexpr auto proj =
				len == 3? &GladGLContext::UniformMatrix3fv
						: &GladGLContext::UniformMatrix4fv;

			const auto& func = ((m_gl.ptr())->*proj);
			func(get_location(name), 1, transpose, reinterpret_cast<const f32*>(&value));
		}

		template<uniform_table_contain_type Type>
		void set(const Str& name, std::span<Type const> values) noexcept(!is_debug)
		{
			using E = UniformTable::Find<Type>;
			using Ptr = const E::UniformType*;

			const auto& func = ((m_gl.ptr())->*E::function);
			func(get_location(name), values.size(), reinterpret_cast<Ptr>(values.data()));
		}

		template<auto len>
		requires (len == 3 || len == 4)
		void set(const Str& name, std::span<Matrix<len, len, f32> const> values,
				 bool transpose = false) noexcept(!is_debug)
		{
			static constexpr auto proj =
				len == 3? &GladGLContext::UniformMatrix3fv
						: &GladGLContext::UniformMatrix4fv;

			const auto& func = ((m_gl.ptr())->*proj);
			func(get_location(name), values.size(), transpose,
				 reinterpret_cast<const f32*>(values.data()));
		}

	private:
		void destroy() noexcept
		{
			if (valid())
			{
				m_gl->DeleteProgram(m_id);
				m_locations.clear();
			}

			m_id = 0;
		}

		GLint get_location(const Str& name) noexcept(!is_debug)
		{
			auto itr = m_locations.find(name);

			if (itr == m_locations.end()) NYAAN_UNLIK
			{
				auto loc = m_gl->GetUniformLocation(get(), name.c_str());

				if constexpr (is_debug)
					if (loc == -1) NYAAN_UNLIK
						throw Error::Exception(name, Error::Code::no_uniform_location);

				return m_locations[name] = loc;
			}

			return itr->second;
		}

		PtrContainer<Context> m_context;
		PtrContainer<const GladGLContext> m_gl;

		GLuint m_id = 0;
		std::unordered_map<Str, GLint> m_locations;

		Layout m_layout;
	};
}
