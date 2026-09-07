module;

#include <memory>

export module Nyaanwork.Core.Utils.UPtr;

import Nyaanwork.Core.Utils.FunctionTraits;

namespace Nyaanwork::Utils
{
	template<typename Who, typename Res, GetFunctionPtr<false, Res, Who*> function>
	struct Deleter
	{
		using FunctionTraits = Utils::FunctionTraits<function>;

		void operator()(Who* ptr)
			{ function(ptr); }
	};

	export template<typename Who, auto function,
					typename Res = FunctionTraits<function>::Result,
					GetFunctionPtr<false, Res, Who*> check_function = function>
	using GetUPtr = std::unique_ptr<Who, Deleter<Who, Res, check_function>>;
}

export namespace Nyaanwork
{
	using namespace Utils;
}
