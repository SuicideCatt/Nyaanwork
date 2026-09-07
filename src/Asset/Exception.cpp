module;

#include <Core/Defines.hpp>

#include <array>

export module Nyaanwork.Asset.Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Exception;

export namespace Nyaanwork::Asset::Error
{
	enum class Code : ErrorCodeMask
	{
		no_error = 0,

		no_image,
		wrong_resolution,
		wrong_resolution_or_offset,
	};
}

export template<>
struct std::is_error_code_enum<Nyaanwork::Asset::Error::Code>
	: public std::true_type {};

export namespace Nyaanwork::Asset::Error
{
	struct Category : Nyaanwork::BaseCategory<Code>
	{
		static constexpr std::array errors = {
			ErrorInfo{Code::no_error, Condition::no_error, "No error"},
			ErrorInfo{Code::no_image, Condition::invalid, "No image"},
			ErrorInfo{
				Code::wrong_resolution,
				Condition::out_of_range,
				"Wrong resolution"
			},
			ErrorInfo{
				Code::wrong_resolution_or_offset,
				Condition::out_of_range,
				"Wrong resolution or resolution"
			},
		};

		const char* name() NYAAN_CNOEX override
			{ return "Nyaanwork::Asset"; }

		Str message(int errc) const override
		{
			check_error_infos(errors);
			return message_helper(errors, errc);
		}

		bool equivalent(int i, const error_condition& c) NYAAN_CNOEX override
			{ return equivalent_helper(errors, i, c); }

		error_condition to_condition(int i) NYAAN_CNOEX override
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
