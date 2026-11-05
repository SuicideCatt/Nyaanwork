module;

#include <Core/Defines.hpp>
#include <Nyaanwork/HighOpenGL.hpp>

#include <OpenGL/Core330/gl.h>

#include <format>
#include <span>
#include <utility>
#include <vector>

export module Nyaanwork.HighOpenGL.Core330:Texture2D;
import :Checks;
import :Context;

import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Ptr;
import Nyaanwork.HighOpenGL.Exception;

export namespace Nyaanwork::HighOpenGL::Core330
{
	struct Filter
	{
		enum class Type : u8
		{
			linear, nearest
		};

		Type mag = Type::linear;
		Opt<Type> min = nullopt; // nullopt: use mag
		Opt<Type> mipmaping = nullopt; // nullopt: no mipmaping
	};

	struct Wraping
	{
		enum class Wrap : u8
		{
			repeat, mirrored_repeat,
			clamp_to_edge, clamp_to_border,
		};

		Wrap x = Wrap::repeat;
		Wrap y = Wrap::repeat;
	};

	enum class TextureTarget : u8
	{
		image, framebuffer
	};
}

namespace Nyaanwork::HighOpenGL::Core330
{
	constexpr PairOf<GLenum> to_gl(Filter filter)
	{
		PairOf<GLenum> res;
		auto& [mag, min] = res;

		auto type = [](Filter::Type type,
					   GLenum linear = GL_LINEAR,
					   GLenum nearest = GL_NEAREST) -> GLenum
		{
			switch (type)
			{
			case Filter::Type::linear:
				return linear;
			case Filter::Type::nearest:
				return nearest;
			}
		};

		mag = type(filter.mag);

		auto mipmap_types =
			[type](const Opt<Filter::Type>& mipmaping_type) -> PairOf<GLenum>
			{
				#define TYPE(mode) \
					type(*mipmaping_type, \
						GL_##mode##_MIPMAP_LINEAR, GL_##mode##_MIPMAP_NEAREST)

				if (!mipmaping_type)
					return {GL_LINEAR, GL_NEAREST};

				return {TYPE(LINEAR), TYPE(NEAREST)};

				#undef TYPE
			};

		auto [min_linear, min_nearest] = mipmap_types(filter.mipmaping);
		min = type(filter.min.value_or(filter.mag), min_linear, min_nearest);

		return res;
	}

	constexpr PairOf<GLenum> to_gl(Wraping wrap)
	{
		using Wrap = Wraping::Wrap;

		auto get = [](Wrap wrap) -> GLenum
		{
			switch (wrap)
			{
			case Wrap::repeat:
				return GL_REPEAT;
			case Wrap::mirrored_repeat:
				return GL_MIRRORED_REPEAT;
			case Wrap::clamp_to_edge:
				return GL_CLAMP_TO_EDGE;
			case Wrap::clamp_to_border:
				return GL_CLAMP_TO_BORDER;
			}
		};

		return {get(wrap.x), get(wrap.y)};
	}

	template<Asset::Pixels::is_pixel Pixel>
	constexpr GLenum to_gl()
	{
		static constexpr std::array formats = {
			GL_RED, GL_RG, GL_RGB, GL_RGBA
		};
		return formats[static_cast<u8>(Pixel::channels)];
	}

	template<typename Vec>
	requires (std::same_as<Vec, vec2<f32>> || std::same_as<Vec, vec3<f32>>
			  || std::same_as<Vec, vec4<f32>>)
	constexpr PairOf<GLenum> to_gl()
	{
		static constexpr std::array formats = {
			PairOf<GLenum>(GL_RG32F, GL_RG),
			PairOf<GLenum>(GL_RGB32F, GL_RGB),
			PairOf<GLenum>(GL_RGBA32F, GL_RGBA)
		};
		return formats[Vec::length()-2];
	}

	template<typename T>
	requires (std::same_as<T, f32> || std::same_as<T, vec2<f32>>
			  || std::same_as<T, vec3<f32>> || std::same_as<T, vec4<f32>>)
	constexpr PairOf<GLenum> to_gl()
	{
		if constexpr (std::same_as<f32, T>)
			return {GL_R32F, GL_RED};
		else
			return vec_to_gl_format<T>();
	}

	class Texture2D_Impl
	{
	public:
		struct Info
		{
			Filter filter;
			Wraping wraping;
			TextureTarget target = TextureTarget::image;
		};

		struct DataInfo
		{
			vec2<u16> resolution;

			GLenum format;

			struct {
				GLenum format;
				GLenum type;
				const void* data;
			} memory;
		};

		Texture2D_Impl() = default;

		Texture2D_Impl(PtrContainer<Context> context, const Info& info,
					   const DataInfo& data_info)
			: p_context(context), p_gl(context->gl()), p_target(info.target)
		{
			auto& gl = p_context->gl();

			gl.GenTextures(1, &p_id);

			wraping<true, false>(info.wraping);
			filter<false, false>(info.filter);

			update<false>(data_info);
		}

		Texture2D_Impl(const Texture2D_Impl&) = delete;
		Texture2D_Impl& operator=(const Texture2D_Impl&) = delete;

		Texture2D_Impl(Texture2D_Impl&& oth) noexcept
			: p_context(std::move(oth.p_context)),
			  p_gl(std::move(oth.p_gl)),
			  p_id(std::exchange(oth.p_id, 0)),
			  p_resolution(std::exchange(oth.p_resolution, {})),
			  p_filter(std::exchange(oth.p_filter, {})),
			  p_wrap(std::exchange(oth.p_wrap, {})) {}

		Texture2D_Impl& operator=(Texture2D_Impl&& oth) noexcept
		{
			if (&oth != this)
			{
				destroy();

				p_context = std::move(oth.p_context);
				p_gl = std::move(oth.p_gl);
				p_id = std::exchange(oth.p_id, 0);
				p_resolution = std::exchange(oth.p_resolution, {});
				p_filter = std::exchange(oth.p_filter, {});
				p_wrap = std::exchange(oth.p_wrap, {});
			}

			return *this;
		}

		~Texture2D_Impl()
			{ destroy(); }

		PtrContainer<Context> context() NYAAN_CNOEX
			{ return p_context; }

		bool valid() NYAAN_CNOEX
			{ return p_id != 0 && p_id != static_cast<GLuint>(-1); }

		GLuint get() NYAAN_CNOEX(!is_debug)
		{
			if constexpr (is_debug)
				if (!valid()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			return p_id;
		}

		Filter filter() NYAAN_CNOEX
			{ return p_filter; }
		template<bool bind = true, bool unbind = true>
		void filter(Filter filter) noexcept(!is_debug)
		{
			if constexpr (bind)
				this->bind<false>(0);

			auto [mag, min] = to_gl(p_filter = filter);

			auto& set_param = p_gl->TexParameteri;
			set_param(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
			set_param(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);

			if constexpr (unbind)
				p_gl->BindTexture(GL_TEXTURE_2D, 0);
		}

		Wraping wraping() NYAAN_CNOEX
			{ return p_wrap; }
		template<bool bind = true, bool unbind = true>
		void wraping(Wraping wraping) noexcept(!is_debug)
		{
			if constexpr (bind)
				this->bind<false>(0);

			auto [x, y] = to_gl(p_wrap = wraping);

			auto& set_param = p_gl->TexParameteri;
			set_param(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, x);
			set_param(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, y);

			if constexpr (unbind)
				p_gl->BindTexture(GL_TEXTURE_2D, 0);
		}

		TextureTarget target() NYAAN_CNOEX
			{ return p_target; }

		template<bool bind = true, bool unbind = true>
		void update(const DataInfo& info)
		{
			if constexpr (bind)
				this->bind<false>(0);

			auto& [resolution, format, memory] = info;

			using src_loc = Error::Exception::src_loc;
			auto check = [&resolution](u16 limit, src_loc src = src_loc::current())
			{
				if (!(resolution.x <= limit) || !(resolution.y <= limit)) NYAAN_UNLIK
				{
					auto msg = std::format("texture_size: !({} <= {})",
										   resolution, vec2<u16>(limit));
					throw Error::Exception(msg, Error::Code::limit_exceeded, src);
				}
			};

			if (p_target == TextureTarget::framebuffer)
			{
				auto limit = [this]{
					#ifdef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
						return p_context->limits().texture_size;
					#else
						return p_context->dynamic_limits().texture_size;
					#endif
				}();

				check(limit);
			}
			else if constexpr (Context::dynamic_limits_mode)
			{
				check(p_context->limits().texture_size);
			}
			#ifndef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
			else if constexpr (is_debug)
			{
				check(Context::limits().texture_size);
			}
			#endif

			auto& gl = p_context->gl();
			auto& [w, h] = resolution;
			gl.TexImage2D(
				GL_TEXTURE_2D, 0, format,
				w, h, 0, memory.format, memory.type, memory.data
			);

			if (p_filter.mipmaping)
				gl.GenerateMipmap(GL_TEXTURE_2D);

			if constexpr (unbind)
				gl.BindTexture(GL_TEXTURE_2D, 0);
		}

		template<bool active_slot = true>
		void bind(u8 slot) noexcept(!Context::dynamic_limits_mode && !is_debug)
		{
			auto& gl = *p_gl;

			if constexpr (active_slot)
			{
				check_texture_slot(*p_context, slot);
				gl.ActiveTexture(GL_TEXTURE0 + slot);
			}

			gl.BindTexture(GL_TEXTURE_2D, get());
		}

		template<bool bind = true, bool unbind = true>
		void read(GLenum format, GLenum type, void* data) noexcept(!is_debug)
		{
			if constexpr (bind)
				this->bind<false>(0);

			auto& gl = p_context->gl();
			gl.GetTexImage(GL_TEXTURE_2D, 0, format, type, data);

			if constexpr (unbind)
				gl.BindTexture(GL_TEXTURE_2D, 0);
		}

	protected:
		void destroy() noexcept
		{
			if (valid())
				p_gl->DeleteTextures(1, &p_id);
			p_id = 0;
		}

		PtrContainer<Context> p_context;
		PtrContainer<const GladGLContext> p_gl;
		GLuint p_id = 0;

		vec2<u16> p_resolution;

		Filter p_filter;
		Wraping p_wrap;
		TextureTarget p_target;
	};
}

export namespace Nyaanwork::HighOpenGL::Core330
{
	struct DepthStencil {};

	template<typename>
	class Texture2D;

	// Depth buffer
	template<>
	class Texture2D<DepthStencil> final : private Texture2D_Impl
	{
	public:
		using Info = Texture2D_Impl::Info;

		Texture2D() = default;

		Texture2D(PtrContainer<Context> context, const Info& info,
				  vec2<u16> resolution, std::span<f32 const> memory)
			: Texture2D_Impl(context, info, {
					.resolution = resolution,
					.format = GL_DEPTH32F_STENCIL8,
					.memory = {
						.format = GL_DEPTH_COMPONENT,
						.type = GL_FLOAT,
						.data = memory.data(),
					},
				}) {}

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;

		Texture2D(Texture2D&&) = default;
		Texture2D& operator=(Texture2D&&) = default;

		~Texture2D() = default;

		using Texture2D_Impl::context;

		using Texture2D_Impl::valid;
		using Texture2D_Impl::get;

		using Texture2D_Impl::filter;
		using Texture2D_Impl::wraping;

		template<bool bind = true, bool unbind = true>
		void update(vec2<u16> resolution, std::span<f32 const> memory) noexcept(!is_debug)
		{
			Texture2D_Impl::update<bind, unbind>({
				.resolution = resolution,
				.format = GL_DEPTH32F_STENCIL8,
				.memory = {
					.format = GL_DEPTH_COMPONENT,
					.type = GL_FLOAT,
					.data = memory.data(),
				},
			});
		}

		using Texture2D_Impl::bind;

		template<bool bind = true, bool unbind = true>
		std::vector<f32> read_depth() noexcept(!is_debug)
		{
			std::vector<f32> image(p_resolution.x * p_resolution.y);

			Texture2D_Impl::read<bind, unbind>(
				GL_DEPTH_COMPONENT, GL_FLOAT, image.data()
			);

			return image;
		}

		template<bool bind = true, bool unbind = true>
		Asset::Image<Asset::Pixels::R> read_stencil() noexcept(!is_debug)
		{
			Asset::Image<Asset::Pixels::R> image(p_resolution);

			Texture2D_Impl::read<bind, unbind>(
				GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, image.data()
			);

			return image;
		}
	};

	// HDR
	template<typename Type>
	requires (std::same_as<Type, f32> || std::same_as<Type, vec2<f32>>
			  || std::same_as<Type, vec3<f32>> || std::same_as<Type, vec4<f32>>)
	class Texture2D<Type> final : private Texture2D_Impl
	{
	private:
		static constexpr PairOf<GLenum> format = to_gl<Type>();

	public:
		using Info = Texture2D_Impl::Info;

		Texture2D() = default;

		Texture2D(PtrContainer<Context> context, const Info& info,
				  vec2<u16> resolution, std::span<f32 const> memory)
			: Texture2D_Impl(context, info, {
					.resolution = resolution,
					.format = format.first,
					.memory = {
						.format = format.second,
						.type = GL_FLOAT,
						.data = memory.data(),
					},
				}) {}

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;

		Texture2D(Texture2D&&) = default;
		Texture2D& operator=(Texture2D&&) = default;

		~Texture2D() = default;

		using Texture2D_Impl::context;

		using Texture2D_Impl::valid;
		using Texture2D_Impl::get;

		using Texture2D_Impl::filter;
		using Texture2D_Impl::wraping;

		template<bool bind = true, bool unbind = true>
		void update(vec2<u16> resolution, std::span<f32 const> memory) noexcept(!is_debug)
		{
			Texture2D_Impl::update<bind, unbind>({
				.resolution = resolution,
				.format = format.first,
				.memory = {
					.format = format.second,
					.type = GL_FLOAT,
					.data = memory.data(),
				},
			});
		}

		using Texture2D_Impl::bind;

		template<bool bind = true, bool unbind = true>
		std::vector<Type> read() noexcept(!is_debug)
		{
			std::vector<Type> image(p_resolution.x * p_resolution.y);

			Texture2D_Impl::read<bind, unbind>(
				format.second, GL_FLOAT, image.data()
			);

			return image;
		}
	};

	template class Texture2D<f32>;
	template class Texture2D<vec2<f32>>;
	template class Texture2D<vec3<f32>>;
	template class Texture2D<vec4<f32>>;

	template<Asset::Pixels::is_pixel Pixel>
	class Texture2D<Pixel> final : private Texture2D_Impl
	{
	private:
		static constexpr auto format = to_gl<Pixel>();

	public:
		using Info = Texture2D_Impl::Info;

		Texture2D() = default;

		Texture2D(PtrContainer<Context> context, const Info& info,
				  vec2<u16> resolution, std::span<Pixel const> memory)
			: Texture2D_Impl(context, info, {
					.resolution = resolution,
					.format = format,
					.memory = {
						.format = format,
						.type = GL_UNSIGNED_BYTE,
						.data = memory.data(),
					},
				}) {}

		Texture2D(PtrContainer<Context> context, const Info& info,
				  Asset::ImageView<const Pixel> image)
			: Texture2D(context, info, image.resolution(), image) {}

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;

		Texture2D(Texture2D&&) = default;
		Texture2D& operator=(Texture2D&&) = default;

		~Texture2D() = default;

		using Texture2D_Impl::context;

		using Texture2D_Impl::valid;
		using Texture2D_Impl::get;

		using Texture2D_Impl::filter;
		using Texture2D_Impl::wraping;

		template<bool bind = true, bool unbind = true>
		void update(vec2<u16> resolution, std::span<Pixel const> memory) noexcept(!is_debug)
		{
			Texture2D_Impl::update<bind, unbind>({
				.resolution = resolution,
				.format = format,
				.memory = {
					.format = format,
					.type = GL_UNSIGNED_BYTE,
					.data = memory.data(),
				},
			});
		}

		template<bool bind = true, bool unbind = true>
		void update(Asset::ImageView<const Pixel> image) noexcept(!is_debug)
			{ update<bind, unbind>(image.resolution(), image); }

		using Texture2D_Impl::bind;

		template<bool bind = true, bool unbind = true>
		Asset::Image<Pixel> read() noexcept(!is_debug)
		{
			Asset::Image<Pixel> image(p_resolution);

			Texture2D_Impl::read<bind, unbind>(
				format, GL_UNSIGNED_BYTE, image.data()
			);

			return image;
		}
	};

	template class Texture2D<Asset::Pixels::R>;
	template class Texture2D<Asset::Pixels::RA>;
	template class Texture2D<Asset::Pixels::RGB>;
	template class Texture2D<Asset::Pixels::RGBA>;
}
