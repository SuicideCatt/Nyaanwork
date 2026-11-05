module;

#include <Core/Defines.hpp>

#include <array>

export module Nyaanwork.HighOpenGL.Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Exception;

export namespace Nyaanwork::HighOpenGL::Error
{
	enum class Code : ErrorCodeMask
	{
		no_error = 0,

		fail_load_glad,

		limits_not_supported,
		limit_exceeded,

		not_valid,

		framebuffer_not_complete,

		fail_compile_shader_module,

		fail_link_shader_program,
		no_uniform_location,

		vertex_descriptor_type_size_not_equal_real_type_size,
	};
}

export template<>
struct std::is_error_code_enum<Nyaanwork::HighOpenGL::Error::Code>
	: public std::true_type {};

export namespace Nyaanwork::HighOpenGL::Error
{
	struct Category : Nyaanwork::BaseCategory<Code>
	{
		static constexpr std::array errors = {
			ErrorInfo{Code::no_error, Condition::no_error, "No error"},

			ErrorInfo{
				Code::fail_load_glad,
				Condition::specific,
				"Fail to load GLAD"
			},

			ErrorInfo{
				Code::limits_not_supported,
				Condition::out_of_range,
				"Limits not supported by hardware"
			},
			ErrorInfo{
				Code::limit_exceeded,
				Condition::out_of_range,
				"Limit exceeded"
			},

			ErrorInfo{
				Code::not_valid,
				Condition::invalid,
				"Not valid"
			},

			ErrorInfo{
				Code::framebuffer_not_complete,
				Condition::specific,
				"Framebuffer not complete"
			},

			ErrorInfo{
				Code::fail_compile_shader_module,
				Condition::specific,
				"Fail to compile shader module"
			},

			ErrorInfo{
				Code::fail_link_shader_program,
				Condition::specific,
				"Fail to link shader program"
			},
			ErrorInfo{
				Code::no_uniform_location,
				Condition::invalid,
				"No uniform location"
			},

			ErrorInfo{
				Code::vertex_descriptor_type_size_not_equal_real_type_size,
				Condition::out_of_range,
				"sizeof(Vertex) != VertexDescriptor::type_size"
			},
		};

		const char* name() NYAAN_CNOEX override
			{ return "Nyaanwork::HighOpenGL"; }

		Str message(int errc) const override
		{
			check_error_infos(errors);
			return message_helper(errors, errc);
		}

		bool equivalent(int i, const error_condition& c) const noexcept override
			{ return equivalent_helper(errors, i, c); }

		error_condition to_condition(int i) const noexcept override
			{ return to_condition_helper(errors, i); }
	};

	const Category& get_category() noexcept
	{
		static Category category;
		return category;
	}

	std::error_code make_error_code(Code err) noexcept
		{ return {static_cast<Nyaanwork::ErrorCodeMask>(err), get_category()}; }

	struct Exception : Nyaanwork::BaseException
	{
		Exception(Code error, src_loc src = src_loc::current())
			: BaseException(error, src) {}

		Exception(StrV msg, Code error,
				  src_loc src = src_loc::current())
			: BaseException(msg, error, src) {}

		Exception(Code error, const Str& shader_error, src_loc src = src_loc::current())
			: BaseException(error, src), shader_error(shader_error) {}

		Exception(StrV msg, Code error, const Str& shader_error,
				  src_loc src = src_loc::current())
			: BaseException(msg, error, src), shader_error(shader_error) {}

		const Str shader_error = "";
	};
}
