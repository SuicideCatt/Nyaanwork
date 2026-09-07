module;

#include <Core/Defines.hpp>

#include <array>

export module Nyaanwork.WindowSystem.OpenGL:Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Exception;

export namespace Nyaanwork::WindowSystem::OpenGL::Error
{
	enum class Code : ErrorCodeMask
	{
		no_error = 0,

		cannot_get_egl_display,
		fail_to_init_elg_display,
		cannot_bind_api,

		opengl32_not_loaded,
		no_wgl_function,

		already_created,
		fail_to_create,

		wrong_version,
	};
}

export template<>
struct std::is_error_code_enum<Nyaanwork::WindowSystem::OpenGL::Error::Code>
	: public std::true_type {};

export namespace Nyaanwork::WindowSystem::OpenGL::Error
{
	struct Category : Nyaanwork::BaseCategory<Code>
	{
		static constexpr std::array errors = {
			ErrorInfo{Code::no_error, Condition::no_error, "No error"},
			ErrorInfo{
				Code::cannot_get_egl_display,
				Condition::invalid,
				"Can't get EGL display"
			},
			ErrorInfo{
				Code::fail_to_init_elg_display,
				Condition::invalid,
				"Fail to init EGL display"
			},
			ErrorInfo{
				Code::cannot_bind_api,
				Condition::out_of_range,
				"Can't bind API"
			},

			ErrorInfo{
				Code::opengl32_not_loaded,
				Condition::not_exist,
				"OpenGL32 not loaded"
			},
			ErrorInfo{
				Code::no_wgl_function,
				Condition::not_exist,
				"no WGL function"
			},

			ErrorInfo{Code::already_created, Condition::already, "Already created"},
			ErrorInfo{Code::fail_to_create, Condition::invalid, "Fail to create"},
			ErrorInfo{Code::wrong_version, Condition::out_of_range, "Wrong version"},
		};

		const char* name() NYAAN_CNOEX override
			{ return "Nyaanwork::WindowSystem::OpenGL"; }

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
	};
}
