module;

#include <type_traits>

export module Nyaanwork.Core.Utils.ClearType;

namespace Nyaanwork::Utils
{
	template<typename Type>
	using RemoveCV = std::remove_cv_t<Type>;

	template<typename Type>
	using RemoveAllExtents = std::remove_all_extents_t<Type>;

	template<typename Type>
	using RemovePointer = std::remove_pointer_t<Type>;

	template<typename Type>
	using RemoveReference = std::remove_reference_t<Type>;

	export template<typename Type>
	using ClearType =
		RemoveCV<RemoveAllExtents<RemovePointer<RemoveReference<Type>>>>;
}

export namespace Nyaanwork
{
	using namespace Utils;
}
