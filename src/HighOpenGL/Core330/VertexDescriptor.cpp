module;

#include <Core/Defines.hpp>

#include <OpenGL/Core330/gl.h>

#include <vector>

export module Nyaanwork.HighOpenGL.Core330:VertexDescriptor;

import Nyaanwork.Core.Types;

export namespace Nyaanwork::HighOpenGL::Core330
{
	struct VertexDescriptor
	{
		struct AttributeDescriptor
		{
			enum class Type : u8
			{
				i8, u8,
				i16, u16,
				i32, u32,
				f32, f64,
			};

			u16 offset = 0;
			u16 count = 1;
			Type type = Type::f32;
			bool normalized = false;
		};

		u16 type_size = 0;
		std::vector<AttributeDescriptor> attributes;

		bool operator==(const VertexDescriptor& b) NYAAN_CNOEX
		{
			if (type_size != b.type_size
				|| attributes.size() != b.attributes.size()) NYAAN_UNLIK
			{
				return false;
			}

			for (usize i = 0; i < attributes.size(); ++i)
			{
				auto& a_attr = attributes[i];
				auto& b_attr = b.attributes[i];

				if (a_attr.offset != b_attr.offset || a_attr.count != b_attr.count
					|| a_attr.type != b_attr.type || a_attr.normalized != b_attr.normalized)
				{
					return false;
				}
			}

			return true;
		}
	};
}
