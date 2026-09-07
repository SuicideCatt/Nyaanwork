module;

#include <Core/Defines.hpp>

#include <array>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Exception;

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error
{
	enum class Code : ErrorCodeMask
	{
		no_error = 0,

		fail_to_open_xlib_display,
		cannot_get_xcb_connection,

		fail_to_create_id,

		extension_not_supported,

		fail_to_init_xkb,
		fail_to_get_core_keyboard,
		fail_to_create_xkb_context,
		fail_to_create_xkb_keymap,
		fail_to_create_xkb_state,

		fail_to_create_xcb_cursor_context,

		wrong_format,

		drag_and_drop_alredy_begined,
	};
}

export template<>
struct std::is_error_code_enum<Nyaanwork::WindowSystem::Backends::Linux::XCB::Error::Code>
	: public std::true_type {};

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error
{
	struct Category : Nyaanwork::BaseCategory<Code>
	{
		static constexpr std::array errors = {
			ErrorInfo{Code::no_error, Condition::no_error, "No error"},
			ErrorInfo{
				Code::fail_to_open_xlib_display,
				Condition::invalid,
				"Fail to open Xlib display"
			},
			ErrorInfo{
				Code::cannot_get_xcb_connection,
				Condition::invalid,
				"Can't get XCB connection"
			},
			ErrorInfo{
				Code::fail_to_create_id,
				Condition::invalid,
				"Fail to create id"
			},
			ErrorInfo{
				Code::extension_not_supported,
				Condition::invalid,
				"Exception not supported"
			},
			ErrorInfo{
				Code::fail_to_init_xkb,
				Condition::invalid,
				"Fail to init XKB"
			},
			ErrorInfo{
				Code::fail_to_get_core_keyboard,
				Condition::invalid,
				"Fail to get core keyboard"
			},
			ErrorInfo{
				Code::fail_to_create_xkb_context,
				Condition::invalid,
				"Fail to create XKB context"
			},
			ErrorInfo{
				Code::fail_to_create_xkb_keymap,
				Condition::invalid,
				"Fail to create XKB keymap"
			},
			ErrorInfo{
				Code::fail_to_create_xkb_state,
				Condition::invalid,
				"Fail to create XKB state"
			},
			ErrorInfo{
				Code::fail_to_create_xcb_cursor_context,
				Condition::invalid,
				"Fail to create XCB-Cursor context"
			},
			ErrorInfo{Code::wrong_format, Condition::wrong_type, "Wrong format"},
			ErrorInfo{
				Code::drag_and_drop_alredy_begined,
				Condition::already,
				"D&D alredy begined"
			},
		};

		const char* name() NYAAN_CNOEX override
			{ return "Nyaanwork::WindowSystem::Backends::Linux::XCB"; }

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
