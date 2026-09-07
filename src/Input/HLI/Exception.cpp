module;

#include <Core/Defines.hpp>

#include <array>

export module Nyaanwork.Input.HLI:Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Exception;

export namespace Nyaanwork::Input::HLI_Error
{
	enum class Code : ErrorCodeMask
	{
		no_error = 0,

		action_already_added,
		action_not_found,

		axis_already_added,
		axis_not_found,

		slot_index_not_less_than_max_combinations_count
	};
}

export template<>
struct std::is_error_code_enum<Nyaanwork::Input::HLI_Error::Code>
	: public std::true_type {};

export namespace Nyaanwork::Input::HLI_Error
{
	struct Category : Nyaanwork::BaseCategory<Code>
	{
		static constexpr std::array errors = {
			ErrorInfo{Code::no_error, Condition::no_error, "No error"},
			ErrorInfo{
				Code::action_already_added, Condition::already,
				"Action already added"
			},
			ErrorInfo{
				Code::action_not_found, Condition::not_found,
				"Action not found"
			},
			ErrorInfo{
				Code::axis_already_added, Condition::already,
				"Axis already added"
			},
			ErrorInfo{
				Code::axis_not_found, Condition::not_found,
				"Axis not found"
			},
			ErrorInfo{
				Code::slot_index_not_less_than_max_combinations_count,
				Condition::out_of_range,
				"Slot index not less than max combinations count"
			},
		};

		const char* name() NYAAN_CNOEX override
			{ return "Nyaanwork::Input::HLI"; }

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
