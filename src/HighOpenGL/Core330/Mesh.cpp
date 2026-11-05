module;

#include <Core/Defines.hpp>

#include <OpenGL/Core330/gl.h>

#include <format>
#include <span>
#include <utility>
#include <vector>

export module Nyaanwork.HighOpenGL.Core330:Mesh;
import :Context;
import :VertexDescriptor;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Ptr;
import Nyaanwork.HighOpenGL.Exception;

export namespace Nyaanwork::HighOpenGL::Core330
{
	enum class PrimitiveTopology : u8
	{
		points,

		lines, line_strip,
		lines_adjacency, line_strip_adjacency,

		triangles, triangle_strip, triangle_fan,
		triangles_adjacency, triangle_strip_adjacency,
	};
}

namespace Nyaanwork::HighOpenGL::Core330
{
	constexpr GLenum to_gl(PrimitiveTopology topology) noexcept
	{
		switch (topology)
		{
		#define CASE(name, gl) \
			case PrimitiveTopology::name: \
				return GL_##gl

		CASE(points, POINTS);

		CASE(lines, LINES);
		CASE(line_strip, LINE_STRIP);
		CASE(lines_adjacency, LINES_ADJACENCY);
		CASE(line_strip_adjacency, LINE_STRIP_ADJACENCY);

		CASE(triangles, TRIANGLES);
		CASE(triangle_strip, TRIANGLE_STRIP);
		CASE(triangle_fan, TRIANGLE_FAN);
		CASE(triangles_adjacency, TRIANGLES_ADJACENCY);
		CASE(triangle_strip_adjacency, TRIANGLE_STRIP_ADJACENCY);

		#undef CASE
		}
	}

	template<typename Index>
	requires (std::same_as<Index, u32> || std::same_as<Index, u16> || std::same_as<Index, u8>)
	constexpr GLenum to_gl() noexcept
	{
		static_assert(std::same_as<u32, GLuint>, "u32 != GLuint");
		static_assert(std::same_as<u16, GLushort>, "u16 != GLushort");
		static_assert(std::same_as<u8, GLubyte>, "u8 != GLubyte");

		if constexpr (std::same_as<Index, u32>)
			return GL_UNSIGNED_INT;
		else if constexpr (std::same_as<Index, u16>)
			return GL_UNSIGNED_SHORT;
		else if constexpr (std::same_as<Index, u8>)
			return GL_UNSIGNED_BYTE;
		else
			static_assert(false, "type not supported");
	}

	template<GLenum target, bool dynamic>
	struct BufferContainer;

	template<GLenum target>
	struct BufferContainer<target, false>
	{
		BufferContainer() = default;

		BufferContainer(const BufferContainer&) = delete;
		BufferContainer& operator=(const BufferContainer&) = delete;

		BufferContainer(BufferContainer&& oth) noexcept
			: id(std::exchange(oth.id, 0)),
			  size(std::exchange(oth.size, 0)) {}

		BufferContainer& operator=(BufferContainer&& oth) noexcept
		{
			if (&oth != this)
			{
				id = std::exchange(oth.id, 0);
				size = std::exchange(oth.size, 0);
			}

			return *this;
		}

		GLuint id = 0;
		usize size = 0;
	};

	template<GLenum target>
	struct BufferContainer<target, true>
	{
		BufferContainer() = default;

		BufferContainer(const BufferContainer&) = delete;
		BufferContainer& operator=(const BufferContainer&) = delete;

		BufferContainer(BufferContainer&& oth) noexcept
			: id(std::exchange(oth.id, 0)),
			  size(std::exchange(oth.size, 0)),
			  capacity(std::exchange(oth.capacity, 0)) {}

		BufferContainer& operator=(BufferContainer&& oth) noexcept
		{
			if (&oth != this)
			{
				id = std::exchange(oth.id, 0);
				size = std::exchange(oth.size, 0);
				capacity = std::exchange(oth.capacity, 0);
			}

			return *this;
		}

		GLuint id = 0;
		usize size = 0;
		usize capacity = 0;
	};

	template<GLenum target, bool dynamic>
	class Buffer : private BufferContainer<target, dynamic>
	{
	private:
		using Container = Buffer::BufferContainer;

	public:
		Buffer() = default;

		void bind(const GladGLContext& gl)
			{ gl.BindBuffer(target, id); }

		template<typename Type>
		void update(const GladGLContext& gl, std::span<Type const> buffer)
		{
			if constexpr (dynamic)
			{
				auto& [id, size, cap] = static_cast<Container&>(*this);
				auto new_cap = cap;

				if (buffer.size() > cap)
					new_cap = static_cast<usize>(static_cast<f64>(buffer.size()) * 1.5);
				else if (buffer.size() < (cap / 2))
					new_cap = buffer.size();

				if (new_cap != cap)
				{
					cap = new_cap;
					gl.BufferData(target, cap*sizeof(Type), nullptr, GL_DYNAMIC_DRAW);
				}

				gl.BufferSubData(target, 0, buffer.size_bytes(), buffer.data());
				size = buffer.size();
			}
			else
			{
				if (size == buffer.size())
					gl.BufferSubData(target, 0, buffer.size_bytes(), buffer.data());
				else
					gl.BufferData(target, buffer.size_bytes(), buffer.data(), GL_STATIC_DRAW);
				size = buffer.size();
			}
		}

		using Container::id;
		using Container::size;
	};

	template<typename Vertex>
	void apply_attributes(const GladGLContext& gl,
						  const VertexDescriptor& vertex_descriptor)
	{
		if (sizeof(Vertex) != vertex_descriptor.type_size)
		{
			throw Error::Exception(
				std::format("sizeof({}) != {}", sizeof(Vertex), vertex_descriptor.type_size),
				Error::Code::vertex_descriptor_type_size_not_equal_real_type_size
			);
		}

		for (usize i = 0; i < vertex_descriptor.attributes.size(); ++i)
		{
			auto& [u16_offset, count, type, normalized] = vertex_descriptor.attributes[i];
			usize offset = u16_offset;

			using T = VertexDescriptor::AttributeDescriptor::Type;
			GLenum gl_type;
			switch (type)
			{
			#define CASE(type, name) \
				case T::type: \
					gl_type = GL_##name; \
					break

			#define CASE_INT(bits, name) \
				CASE(i##bits, name); CASE(u##bits, UNSIGNED_##name)

			#define CASE_FLOAT(bits, name) \
				CASE(f##bits, name)

			CASE_INT(8, BYTE);
			CASE_INT(16, SHORT);
			CASE_INT(32, INT);

			CASE_FLOAT(32, FLOAT);
			CASE_FLOAT(64, DOUBLE);

			#undef CASE_FLOAT
			#undef CASE_INT
			#undef CASE
			}

			gl.EnableVertexAttribArray(i);
			gl.VertexAttribPointer(i, count, gl_type, normalized,
								   vertex_descriptor.type_size,
								   reinterpret_cast<void*>(offset));
		}
	}
}

export namespace Nyaanwork::HighOpenGL::Core330
{
	template<typename Vertex, typename Index, bool dynamic>
	requires (std::same_as<Index, u32> || std::same_as<Index, u16> || std::same_as<Index, u8>)
	class MeshWithIndex
	{
	public:
		MeshWithIndex() = default;

		MeshWithIndex(PtrContainer<Context> context, const VertexDescriptor& vertex_descriptor,
					  PrimitiveTopology topology,
					  std::span<Vertex const> vertices, std::span<Index const> indices)
			: m_context(context), m_gl(context->gl()),
			  m_topology(topology), m_vertex_descriptor(vertex_descriptor)
		{
			auto& gl = context->gl();

			gl.GenVertexArrays(1, &m_id);

			GLuint buffers_array[2] = {};
			gl.GenBuffers(2, buffers_array);
			m_vertices.id = buffers_array[0];
			m_indices.id = buffers_array[1];

			gl.BindVertexArray(get());

			m_vertices.bind(gl);
			m_vertices.update(gl, vertices);

			m_indices.bind(gl);
			m_indices.update(gl, indices);

			apply_attributes<Vertex>(gl, vertex_descriptor);

			gl.BindVertexArray(0);
			gl.BindBuffer(GL_ARRAY_BUFFER, 0);
			gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		template<typename MeshContainer>
		requires requires(const MeshContainer& mesh)
		{
			std::span<Vertex const>(mesh.vertices);
			std::span<Index const>(mesh.indices);
		}
		MeshWithIndex(PtrContainer<Context> context,
					  const VertexDescriptor& vertex_descriptor,
					  PrimitiveTopology topology, const MeshContainer& mesh)
			: MeshWithIndex(context, vertex_descriptor, topology,
							mesh.vertices, mesh.indices) {}

		MeshWithIndex(const MeshWithIndex&) = delete;
		MeshWithIndex& operator=(const MeshWithIndex&) = delete;

		MeshWithIndex(MeshWithIndex&& oth) noexcept
			: m_context(std::move(oth.m_context)),
			  m_gl(std::move(oth.m_gl)),
			  m_id(std::exchange(oth.m_id, 0)),
			  m_vertices(std::move(oth.m_vertices)),
			  m_indices(std::move(oth.m_indices)),
			  m_topology(std::exchange(oth.m_topology, PrimitiveTopology::triangles)),
			  m_vertex_descriptor(std::move(oth.m_vertex_descriptor)) {}

		MeshWithIndex& operator=(MeshWithIndex&& oth) noexcept
		{
			if (&oth != this)
			{
				destroy();

				m_context = std::move(oth.m_context);
				m_gl = std::move(oth.m_gl);
				m_id = std::exchange(oth.m_id, 0);
				m_vertices = std::move(oth.m_vertices);
				m_indices = std::move(oth.m_indices);
				m_topology = std::exchange(oth.m_topology, PrimitiveTopology::triangles);
				m_vertex_descriptor = std::move(oth.m_vertex_descriptor);
			}

			return *this;
		}

		~MeshWithIndex()
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

		Pair<GLuint, usize> vertices() NYAAN_CNOEX
			{ return {m_vertices.id, m_vertices.size}; }

		Pair<GLuint, usize> indices() NYAAN_CNOEX
			{ return {m_indices.id, m_indices.size}; }

		PrimitiveTopology topology() NYAAN_CNOEX
			{ return m_topology; }

		VertexDescriptor vertex_descriptor() NYAAN_CNOEX
			{ return m_vertex_descriptor; }

		void update_indices(std::span<Index const> buffer) noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!valid()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			auto& gl = *m_gl;
			m_indices.bind(gl);
			m_indices.update(gl, buffer);
			gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		void update(PrimitiveTopology topology,
					std::span<Vertex const> vertices,
					std::span<Index const> indices) noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!valid()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			m_topology = topology;

			auto& gl = *m_gl;
			m_vertices.bind(gl);
			m_vertices.update(gl, vertices);
			gl.BindBuffer(GL_ARRAY_BUFFER, 0);

			update_indices(indices);
		}

		template<typename MeshContainer>
		requires requires(const MeshContainer& mesh)
		{
			std::span<Vertex const>(mesh.vertices);
			std::span<Index const>(mesh.indices);
		}
		void update(PrimitiveTopology topology, const MeshContainer& mesh)
			{ update(topology, mesh.vertices, mesh.indices); }

		void bind() noexcept(!is_debug)
			{ m_gl->BindVertexArray(get()); }

		void draw(u32 instances = 1,
				  Opt<usize> indices_count = nullopt) noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!valid()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			m_gl->DrawElementsInstanced(
				to_gl(m_topology),
				indices_count.value_or(m_indices.size), get_index_type<Index>(),
				nullptr, instances
			);
		}

	private:
		void destroy() noexcept
		{
			if (valid())
			{
				auto& gl = m_context->gl();

				GLuint buffers_array[] = {m_vertices.id, m_indices.id};
				gl.DeleteBuffers(2, buffers_array);
				gl.DeleteVertexArrays(1, &m_id);
			}

			m_id = 0;
		}

		PtrContainer<Context> m_context;
		PtrContainer<const GladGLContext> m_gl;
		GLuint m_id = 0;

		Buffer<GL_ARRAY_BUFFER, dynamic> m_vertices;
		Buffer<GL_ELEMENT_ARRAY_BUFFER, dynamic> m_indices;

		PrimitiveTopology m_topology = PrimitiveTopology::triangles;
		VertexDescriptor m_vertex_descriptor;
	};

	template<typename Vertex, bool dynamic>
	class MeshWithoutIndex
	{
	public:
		MeshWithoutIndex() = default;

		MeshWithoutIndex(PtrContainer<Context> context,
						 const VertexDescriptor& vertex_descriptor,
						 PrimitiveTopology topology, std::span<Vertex const> vertices)
			: m_context(context), m_gl(context->gl()),
			  m_topology(topology), m_vertex_descriptor(vertex_descriptor)
		{
			auto& gl = context->gl();

			gl.GenVertexArrays(1, &m_id);
			gl.GenBuffers(1, &m_vertices.id);

			gl.BindVertexArray(get());

			m_vertices.bind(gl);
			m_vertices.update(gl, vertices);

			apply_attributes<Vertex>(gl, vertex_descriptor);

			gl.BindVertexArray(0);
			gl.BindBuffer(GL_ARRAY_BUFFER, 0);
		}

		MeshWithoutIndex(const MeshWithoutIndex&) = delete;
		MeshWithoutIndex& operator=(const MeshWithoutIndex&) = delete;

		MeshWithoutIndex(MeshWithoutIndex&& oth) noexcept
			: m_context(std::move(oth.m_context)),
			  m_gl(std::move(oth.m_gl)),
			  m_id(std::exchange(oth.m_id, 0)),
			  m_vertices(std::move(oth.m_vertices)),
			  m_topology(std::exchange(oth.m_topology, PrimitiveTopology::triangles)),
			  m_vertex_descriptor(std::move(oth.m_vertex_descriptor)) {}

		MeshWithoutIndex& operator=(MeshWithoutIndex&& oth) noexcept
		{
			if (&oth != this)
			{
				destroy();

				m_context = std::move(oth.m_context);
				m_gl = std::move(oth.m_gl);
				m_id = std::exchange(oth.m_id, 0);
				m_vertices = std::move(oth.m_vertices);
				m_topology = std::exchange(oth.m_topology, PrimitiveTopology::triangles);
				m_vertex_descriptor = std::move(oth.m_vertex_descriptor);
			}

			return *this;
		}

		~MeshWithoutIndex()
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

		Pair<GLuint, usize> vertices() NYAAN_CNOEX
			{ return {m_vertices.id, m_vertices.size}; }

		PrimitiveTopology topology() NYAAN_CNOEX
			{ return m_topology; }

		VertexDescriptor vertex_descriptor() NYAAN_CNOEX
			{ return m_vertex_descriptor; }

		void update(PrimitiveTopology topology,
					std::span<Vertex const> vertices) noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!valid()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			m_topology = topology;

			auto& gl = *m_gl;
			m_vertices.bind(gl);
			m_vertices.update(gl, vertices);
			gl.BindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void bind() noexcept(!is_debug)
			{ m_gl->BindVertexArray(get()); }

		void draw(u32 instances = 1,
				  Opt<usize> vertices_count = nullopt) noexcept(!is_debug)
		{
			if constexpr (is_debug)
				if (!valid()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::not_valid);

			m_gl->DrawArraysInstanced(
				to_gl(m_topology), 0,
				vertices_count.value_or(m_vertices.size), instances
			);
		}

	private:
		void destroy() noexcept
		{
			if (valid())
			{
				auto& gl = m_context->gl();

				gl.DeleteBuffers(1, &m_vertices.id);
				gl.DeleteVertexArrays(1, &m_id);
			}

			m_id = 0;
		}

		PtrContainer<Context> m_context;
		PtrContainer<const GladGLContext> m_gl;
		GLuint m_id = 0;

		Buffer<GL_ARRAY_BUFFER, dynamic> m_vertices;

		PrimitiveTopology m_topology = PrimitiveTopology::triangles;
		VertexDescriptor m_vertex_descriptor;
	};
}

namespace Nyaanwork::HighOpenGL::Core330
{
	template<typename Vertex, typename Index, bool dynamic>
	struct MeshT
	{
		using Type = MeshWithIndex<Vertex, Index, dynamic>;
	};

	template<typename Vertex, bool dynamic>
	struct MeshT<Vertex, void, dynamic>
	{
		using Type = MeshWithoutIndex<Vertex, dynamic>;
	};
}

export namespace Nyaanwork::HighOpenGL::Core330
{
	template<typename Vertex, typename Index, bool dynamic>
	using Mesh = MeshT<Vertex, Index, dynamic>::Type;

	template<typename Vertex, typename Index>
	using StaticMesh = Mesh<Vertex, Index, false>;

	template<typename Vertex, typename Index>
	using DynamicMesh = Mesh<Vertex, Index, true>;
}
