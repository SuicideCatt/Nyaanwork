module;

#include <Core/Defines.hpp>

#include <format>
#include <source_location>
#include <span>
#include <system_error>

export module Nyaanwork.Core.Exception;

import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.FunctionTraits;

namespace Nyaanwork
{
	struct ExceptionDataContainer
	{
		struct FunctionInfo
		{
			std::string_view result, name, arguments;

			FunctionInfo(std::string_view function_name)
			{
				auto f = function_name.find('(');
				auto fa = function_name.substr(0, f);

				auto n = fa.find_first_of(' ');
				auto nt = fa.find_first_of("> ");

				auto l = function_name.find(") [", f);

				if (nt != function_name.npos)
					n = nt;

				name = function_name.substr(n + 1, f - n - 1);

				size_t offset = 0;
				while (name[0] == '&' || name[0] == '*')
				{
					name = name.substr(1);
					++offset;
				}

				if (offset != 0)
					++offset;

				result = function_name.substr(0, n + offset);

				if (l + 1 != function_name.length())
				{
					arguments = function_name.substr(f + 1, l - f - 1);

					if (!arguments.empty() && arguments.back() == ')')
						arguments = arguments.substr(0, arguments.size()-1);
				}
			}
		};

		ExceptionDataContainer(std::string_view fi, std::source_location src)
			: source(src), function_info(fi) {}

		ExceptionDataContainer(std::string_view fi, std::string_view msg,
							   std::source_location src)
			: source(src), function_info(fi), message(msg) {}

		const std::source_location source;
		const FunctionInfo function_info;
		const std::string message;
	};

	void error(const char*);
}

export namespace Nyaanwork
{
	using ErrorCodeMask = MethodTraits<&std::error_code::value>::Result;

	enum class Condition : ErrorCodeMask
	{
		no_error = 0,

		already,
		invalid,
		not_exist,
		not_found,
		out_of_range,

		wrong_type,

		specific,
	};

	struct ConditionCategory : std::error_category
	{
		const char* name() NYAAN_CNOEX override
			{ return "Nyaanwork::Condition"; }

		std::string message(ErrorCodeMask errc) const override
		{
			switch (static_cast<Condition>(errc))
			{
			case Condition::no_error:
				return "No error";
			case Condition::already:
				return "Already";
			case Condition::invalid:
				return "Invalid";
			case Condition::not_exist:
				return "Not exist";
			case Condition::not_found:
				return "Not found";
			case Condition::out_of_range:
				return "Out of range";
			case Condition::wrong_type:
				return "Wrong type";
			case Condition::specific:
				return "Specific error";

			default:
				return "Unknown error code (" + std::to_string(errc) + ')';
			}
		}
	};

	const ConditionCategory& get_error_condition_category() noexcept
	{
		static ConditionCategory category;
		return category;
	}

	std::error_condition make_error_condition(Condition c) noexcept
		{ return {static_cast<ErrorCodeMask>(c), get_error_condition_category()}; }
}

export template<>
struct std::is_error_condition_enum<Nyaanwork::Condition> : true_type {};

export namespace Nyaanwork
{
	template<typename>
	class BaseCategory;

	template<>
	class BaseCategory<void> : public std::error_category
	{
	protected:
		using error_condition = std::error_condition;
		using Condition = Nyaanwork::Condition;

	public:
		virtual ~BaseCategory() = default;

		virtual error_condition to_condition(int i) NYAAN_CNOEX = 0;

	protected:
		BaseCategory() = default;
	};

	template<is_scoped_enum_with_mask<ErrorCodeMask> _Code>
	class BaseCategory<_Code> : public BaseCategory<void>
	{
	protected:
		using BaseCategory<void>::error_condition;
		using BaseCategory<void>::Condition;

	public:
		using Code = _Code;

		virtual ~BaseCategory() = default;

		virtual error_condition to_condition(int i) NYAAN_CNOEX override = 0;

	protected:
		struct ErrorInfo
		{
			Code code;
			Condition condition;
			std::string_view message;
		};

		BaseCategory() = default;

		std::string message_helper(std::span<ErrorInfo const> errs,
								   ErrorCodeMask errc) const
		{
			if (errs.size() > errc)
			{
				auto err = errs[errc].message;
				return {err.data(), err.size()};
			}

			return std::format("Unknown error code ({})", errc);
		}

		bool equivalent_helper(std::span<ErrorInfo const> errs, ErrorCodeMask i,
							   const error_condition& c) NYAAN_CNOEX
		{
			if (errs.size() > i && c.category() == get_error_condition_category())
				return errs[i].condition == c;

			return false;
		}

		error_condition to_condition_helper(std::span<ErrorInfo const> errs,
											ErrorCodeMask i) NYAAN_CNOEX
		{
			if (i >= errs.size())
				return Condition::specific;

			return errs[i].condition;
		}

		template<size_t size>
		consteval static void check_error_infos(std::array<ErrorInfo, size> arr)
		{
			for (int i = 0; i < arr.size(); ++i)
				if (i != static_cast<ErrorCodeMask>(arr[i].code))
					error("Code does not match any array ID.");
		}
	};

	struct BaseException
		: private ExceptionDataContainer, public std::system_error
	{
		using src_loc = std::source_location;

		BaseException(std::error_code code, src_loc src = src_loc::current())
			: ExceptionDataContainer(src.function_name(), src),
			  std::system_error(code, [&]
				{
					return std::format(
						"{} Error in {}({})#{}",
						code.category().name(),
						function_info.name,
						(!function_info.arguments.empty()? "..." : ""),
						src.line()
					);
				}()) {}

		BaseException(std::string_view message, std::error_code code,
					  src_loc src = src_loc::current())
			: ExceptionDataContainer(src.function_name(), message, src),
			  std::system_error(code, [&]
				{
					return std::format(
						"{} Error in {}({})#{}: {}",
						code.category().name(),
						function_info.name,
						(!function_info.arguments.empty()? "..." : ""),
						src.line(), message
					);
				}()) {}

		virtual ~BaseException() = default;

		std::error_condition code_to_condition() NYAAN_CNOEX
		{
			auto c = code();
			auto category = dynamic_cast<const BaseCategory<void>*>(&c.category());
			if (category)
				return category->to_condition(c.value());

			return Condition::specific;
		}

		using ExceptionDataContainer::source;
		using ExceptionDataContainer::function_info;
		using ExceptionDataContainer::message;
	};
}

export namespace std
{
	using std::true_type;
	using std::false_type;
	using std::is_error_code_enum;
	using std::error_code;
};
