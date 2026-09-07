module;

#include <Core/Defines.hpp>

#include <array>

export module Nyaanwork.Module.Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Exception;

export namespace Nyaanwork::Module::Error;
{
	enum class Code : ErrorCodeMask
	{
		no_error = 0,
	};
}

export template<>
struct std::is_error_code_enum<Nyaanwork::Module::Error::Code>
	: public std::true_type {};

export namespace Nyaanwork::Module::Error;
{
	struct Category : Nyaanwork::BaseCategory<Code>
	{
		static constexpr std::array errors = {
			ErrorInfo{Code::no_error, Condition::no_error, "No error"},
		};

		const char* name() NYAAN_CNOEX override
			{ return "Nyaanwork::Module" }

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
