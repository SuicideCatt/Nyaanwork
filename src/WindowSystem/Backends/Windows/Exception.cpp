module;

#include <Core/Defines.hpp>

#include <array>

export module Nyaanwork.WindowSystem.Backends.Windows:Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Exception;

export namespace Nyaanwork::WindowSystem::Backends::Windows::Error
{
	enum class Code : ErrorCodeMask
	{
		no_error = 0,

		cannot_enumarate_modules,

		no_safety_getter,
		instance_count_greater_than_u16_max,
		instance_not_found,
		safety_not_inited,

		fail_to_init_COM,
		fail_to_init_Ole,

		fail_to_register_window_class,

		fail_to_craete_window,
		fail_to_enable_mouse_tracking,
		fail_to_create_icon,
	};
}

export template<>
struct std::is_error_code_enum<Nyaanwork::WindowSystem::Backends::Windows::Error::Code>
	: public std::true_type {};

export namespace Nyaanwork::WindowSystem::Backends::Windows::Error
{
	struct Category : Nyaanwork::BaseCategory<Code>
{
		static constexpr std::array errors = {
			ErrorInfo{Code::no_error, Condition::no_error, "No error"},

			ErrorInfo{
				Code::cannot_enumarate_modules,
				Condition::invalid,
				"Can't enumarate modules"
			},

			ErrorInfo{
				Code::no_safety_getter,
				Condition::not_exist,
				"No safety getter"
			},
			ErrorInfo{
				Code::instance_count_greater_than_u16_max,
				Condition::out_of_range,
				"Instance count > limits<u16>::max()"
			},
			ErrorInfo{
				Code::instance_not_found,
				Condition::not_found,
				"Instance not found"
			},
			ErrorInfo{
				Code::safety_not_inited,
				Condition::invalid,
				"Safety not inited"
			},

			ErrorInfo{
				Code::fail_to_init_COM,
				Condition::invalid,
				"Fail to init COM",
			},
			ErrorInfo{
				Code::fail_to_init_Ole,
				Condition::invalid,
				"Fail to init Ole",
			},

			ErrorInfo{
				Code::fail_to_register_window_class,
				Condition::invalid,
				"Fail to register window class"
			},

			ErrorInfo{
				Code::fail_to_craete_window,
				Condition::invalid,
				"Fail to create window"
			},
			ErrorInfo{
				Code::fail_to_enable_mouse_tracking,
				Condition::invalid,
				"Fail to enable mouse tracking"
			},
			ErrorInfo{
				Code::fail_to_create_icon,
				Condition::invalid,
				"Fail to create icon"
			},
		};

		const char* name() NYAAN_CNOEX override
			{ return "Nyaanwork::WindowSystem::Backends::Windows"; }

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
